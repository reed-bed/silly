// will be using reqwest to query the api and json to parse returned data
// consider passing around some more state to comply with rate limits

use json;
use reqwest;
use reqwest::Url;

use std::{collections::HashMap, thread, time::Duration};

use super::author_tools::{Author, AuthorID};

fn rate_limited_get<T: reqwest::IntoUrl>(url: T) -> reqwest::Result<reqwest::blocking::Response> {
    // 15 requests in 5e9 ns
    let third_second_delay = Duration::new(0, 340000000);
    thread::sleep(third_second_delay);
    reqwest::blocking::get(url)
}

pub fn lookup_author(id: &AuthorID) -> Author {
    let reflink = gen_reflink(id);
    // download author page. consider some error handling
    let auth_text = rate_limited_get(reflink)
        .expect("Failed to download author page")
        .text()
        .expect("No response text");
    //println!("{auth_text}");
    // get their name
    let mut auth_parsed = json::parse(&auth_text).expect("Invalid author json");
    let name = match auth_parsed["metadata"]["name"]["preferred_name"].take_string() {
        Some(n) => n,
        _ => auth_parsed["metadata"]["name"]["value"]
            .take_string()
            .expect("Nameless author"),
    };
    // return object
    Author::new(name)
}

pub fn get_collaborators(id: &AuthorID, since: u32, max_authors: u32) -> HashMap<AuthorID, u32> {
    // query inspire for papers
    let reflink = gen_reflink(id);
    let size = 800;
    let search_query = format!(
        // %24 is $, which parse does not encode for some reason
        "https://inspirehep.net/api/literature?q=de>{since} and authors.record.%24ref:\"{reflink}\""
    );
    let search_query = Url::parse(&search_query).expect("failed to make author search term");
    let lit_text = rate_limited_get(search_query)
        .expect("Failed to query literature")
        .text()
        .expect("No response text");
    let mut auth_parsed = json::parse(&lit_text).expect("Invalid literature json");
    // json array of papers
    let mut results = auth_parsed["hits"]["hits"].take();

    let mut collaboration_table = HashMap::new();

    // get authors of each paper
    for mut paper in results.members_mut() {
        let author_urls = paper["metadata"]["authors"]
            .members_mut()
            .map(|mut x| x["record"]["$ref"].take_string())
            .collect::<Vec<Option<String>>>();

        if author_urls.len() <= max_authors as usize {
            for a in author_urls {
                let coll_id = match a {
                    Some(r) => idstring_from_reflink(&r),
                    _ => panic!("Missing ref"),
                };
                if coll_id != *id {
                    collaboration_table
                        .entry(coll_id)
                        .and_modify(|count| *count += 1)
                        .or_insert(1);
                }
            }
        }
    }

    collaboration_table
}

fn gen_reflink(id: &AuthorID) -> String {
    let AuthorID::RefID(r) = id;
    format!("https://inspirehep.net/api/authors/{r}")
}

fn idstring_from_reflink(reflink: &str) -> AuthorID {
    let s = Url::parse(reflink)
        .expect("Invalid author reflink")
        .path_segments()
        .expect("Invalid author id")
        .last()
        .expect("Invalid author id")
        .to_string();
    AuthorID::RefID(s)
}

// query literature by Hawking:
//authors.record.$ref:"https://inspirehep.net/api/authors/1006450"

////////////////////////////////////////////////////////////////////////////////
// unit tests
////////////////////////////////////////////////////////////////////////////////

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn lookup_hawking() {
        // check that we can query Stephen Hawking
        let hawking_id = AuthorID::RefID("1006450".to_string());
        let hawking = lookup_author(&hawking_id);
        let n = hawking.get_name();
        // assert that we got him
        println!("Found author with name {n}");
        assert!(n == "Stephen W. Hawking");
    }

    #[test]
    fn hawking_collaborators() {
        // his collaborators will not change :( so we can use them as test data
        let hawking_id = AuthorID::RefID("1006450".to_string());
        let hawking_collab = get_collaborators(&hawking_id, 2015, 10);

        let expected_collabs_v = vec![
            ("1003761", 1),
            ("1020130", 1),
            ("2262881", 1),
            ("987332", 3),
            ("993693", 4),
            ("2189347", 1),
            ("1006124", 1),
        ];
        let mut expected_collab = HashMap::new();
        for (n, count) in expected_collabs_v {
            expected_collab.insert(AuthorID::RefID(n.to_string()), count);
        }

        println!("{hawking_collab:?}");
        println!("{expected_collab:?}");

        assert!(hawking_collab == expected_collab);
    }
}
