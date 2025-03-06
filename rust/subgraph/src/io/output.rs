use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize)]
pub struct Vertex {
    pub name: String,
    pub label: String,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Edge {
    pub from: String,
    pub to: String,
    pub from_label: String,
    pub to_label: String,
    pub e_label: String,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Structure {
    pub tid: usize,
    pub vertices: Vec<Vertex>,
    pub edges: Vec<Edge>,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct NodeId {
    pub gid: usize,
    pub nid: String,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Instance {
    pub node_num: usize,
    pub node_ids: Vec<NodeId>,
    pub edges: Vec<Edge>,
}