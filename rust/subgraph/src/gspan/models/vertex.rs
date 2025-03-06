use crate::gspan::models::edge::Edge;
use crate::io::node::Node;

#[derive(Debug, Clone)]
pub struct Vertex {
    pub name: String,
    pub label: String,
    pub edges: Vec<Edge>,
}

impl Vertex {
    pub const NIL_V_LABEL: &str = "<NIL>";

    pub fn new(name: String, label: Option<String>) -> Vertex {
        Vertex {
            name,
            label: match label {
                None => String::new(),
                Some(label) => label,
            },
            edges: Vec::with_capacity(8),
        }
    }

    pub fn push(&mut self, edge: Edge) {
        self.edges.push(edge);
    }

    pub fn to_str_repr(&self) -> String {
        vec![
            "v".to_string(),
            self.name.to_string(),
            self.label.to_string(),
        ]
        .join(" ")
    }

    pub fn from(node: &Node) -> Vertex {
        Vertex::new(node.name.clone(), Some(node.opType.clone()))
    }
}

impl PartialEq for Vertex {
    fn eq(&self, other: &Self) -> bool {
        self.label == other.label
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_create_vertex() {
        let v1 = Vertex::new(String::from("node_1"), None);
        assert_eq!(v1.name, "node_1");
    }

    #[test]
    fn test_add_edge() {
        let mut v1 = Vertex::new(String::from("node_1"), Some(String::from("2")));
        assert_eq!(v1.edges.len(), 0);
        assert_eq!(v1.label,"2");
        let e1 = Edge::new(
            v1.name.clone(),
            String::from("node_2"),
            v1.label.clone(),
            String::from("2"),
            Some(String::from("e_2")),
        );
        v1.push(e1);
        assert_eq!(v1.edges.len(), 1);
        let e = v1.edges.pop().unwrap();
        assert_eq!(v1.edges.len(), 0);
        assert_eq!(&e.from, &v1.name);
        assert_eq!(e.to, "node_2");
        assert_eq!(e.e_label, "e_2");
    }
}
