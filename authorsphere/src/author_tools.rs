use std::collections::HashMap;

use serde::{Deserialize, Serialize};

// there are some complications because AuthorID contains heap-allocated data
// and therefore needs to be owned by something.
// an elegant solution might be to have the AuthorTable own all of the AuthorIDs
#[derive(PartialEq, Eq, Hash, Clone, Debug, Serialize, Deserialize)]
pub enum AuthorID {
    RefID(String),
}

#[derive(Serialize, Deserialize)]
pub struct Author {
    // inspire ID of the author, used to e.g. find their papers
    //id: AuthorID,
    // name of the author
    name: String,
    // the author's collaborators, and the number of papers co-authored
    collaborations: HashMap<AuthorID, u32>,
}

impl Author {
    pub fn new(name: String) -> Author {
        Author {
            collaborations: HashMap::new(),
            name,
        }
    }
    pub fn add_collaboration(&mut self, other_id: &AuthorID, count: u32) {
        self.collaborations
            .entry(other_id.clone())
            .and_modify(|c| (*c += 1))
            .or_insert(1);
    }

    pub fn set_collaborations(&mut self, collaborations: HashMap<AuthorID, u32>) {
        self.collaborations = collaborations;
    }

    pub fn get_name(&self) -> &String {
        &self.name
    }
    pub fn get_collaboration(&self, other_id: &AuthorID) -> Option<&u32> {
        self.collaborations.get(other_id)
    }

    pub fn get_collaborators(&self) -> Vec<AuthorID> {
        self.collaborations
            .keys()
            .cloned()
            .collect::<Vec<AuthorID>>()
    }
}

#[derive(Serialize, Deserialize)]
pub struct AuthorTable {
    // look up an AuthorID to see whether there is an Author struct associated
    author_table: HashMap<AuthorID, Author>,
    // look up an AuthorID to see the graph depth at which they were discovered
    author_depths: HashMap<AuthorID, u32>,
}

impl AuthorTable {
    pub fn new() -> AuthorTable {
        AuthorTable {
            author_table: HashMap::new(),
            author_depths: HashMap::new(),
        }
    }

    pub fn get_depth(&self, id: &AuthorID) -> Option<&u32> {
        self.author_depths.get(id)
    }
    pub fn get_author(&self, id: &AuthorID) -> Option<&Author> {
        self.author_table.get(id)
    }
    pub fn add_author(&mut self, id: &AuthorID, auth: Author, depth: u32) {
        self.author_depths
            .entry(id.clone())
            .and_modify(|d| (*d = if depth < *d { depth } else { *d }))
            .or_insert(depth);
        self.author_table.entry(id.clone()).insert_entry(auth);
    }
    pub fn update_depth(&mut self, id: &AuthorID, depth: u32) {
        self.author_depths
            .entry(id.clone())
            .and_modify(|d| (*d = if depth < *d { depth } else { *d }));
    }

    pub fn all_authors(&self) -> Vec<&AuthorID> {
        self.author_table.keys().collect::<Vec<&AuthorID>>()
    }
}
