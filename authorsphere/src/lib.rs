pub mod author_tools;
pub mod inspire_tools;

use author_tools::{Author, AuthorID, AuthorTable};

pub fn map_collaborators(
    root: &AuthorID,
    depth: u32,
    since: u32,
    max_authors: u32,
    mut table: Option<AuthorTable>,
) -> AuthorTable {
    println!("{root:?}, {depth}");
    let mut table = match table {
        Some(t) => t,
        None => AuthorTable::new(),
    };
    match table.get_depth(root) {
        Some(&d) if d >= depth => table,
        _ => map_collaborators_int(root, depth, since, max_authors, table),
    }
}

fn map_collaborators_int(
    root: &AuthorID,
    depth: u32,
    since: u32,
    max_authors: u32,
    mut table: AuthorTable,
) -> AuthorTable {
    // see whether we already crawled the author
    // if so, no need to download their data from inspire
    if table.get_depth(root).is_none() {
        // get author data from Inspire
        let mut auth = inspire_tools::lookup_author(root);

        // get author collaborations from Inspire
        let auth_collabs = inspire_tools::get_collaborators(root, since, max_authors);

        auth.set_collaborations(auth_collabs);

        // enrol author in table
        table.add_author(root, auth, depth);
    }

    // need them again
    let auth = table.get_author(root).unwrap();

    // depth-first search
    if depth > 0 {
        auth.get_collaborators()
            .into_iter()
            .fold(table, |tab, collab| {
                map_collaborators(&collab, depth - 1, since, max_authors, Some(tab))
            })
    } else {
        table
    }
}

#[cfg(test)]
mod test {
    #[test]
    fn map_test() {
        use super::*;
        let m = map_collaborators(&AuthorID::RefID("1006450".to_string()), 1, 2015, 10, None);
        // to do: implement read out of collaborations for processing
        //panic!("a");
    }
}
