use crate::gspan::models::edge::Edge;
use crate::gspan::models::vertex::Vertex;
use crate::io::model_graph::ModelGraph;
use crate::io::node::Node;
use std::collections::HashMap;
use std::fmt;
use std::fs;

#[derive(Debug)]
pub struct GraphSetParseError {
    message: String,
}

impl fmt::Display for GraphSetParseError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.message)
    }
}

#[derive(Debug, Clone)]
#[allow(dead_code)]
pub struct Graph {
    pub id: usize,
    pub name: String,
    pub edge_size: usize,
    pub directed: bool,
    pub vertices: Vec<Vertex>,
    pub vertex_name_label_map: HashMap<String, String>,
}

impl Graph {
    pub fn new(id: usize, directed: bool) -> Graph {
        Graph {
            id,
            name: String::new(),
            edge_size: 0,
            directed,
            vertices: Vec::with_capacity(32),
            vertex_name_label_map: HashMap::new(),
        }
    }

    pub fn insert_vertex(&mut self, name: &str, label: &str) {
        self.vertex_name_label_map
            .insert(name.to_string(), label.to_string());
        let vertex = Vertex::new(name.to_string(), Some(label.to_string()));
        self.vertices.push(vertex);
    }

    pub fn build_edge(&mut self, data: Vec<(String, String, Option<String>)>) {
        for (from, to, e_label) in data {
            if let Some(f_vertex) = self.vertices.iter_mut().find(|x| x.name == from) {
                if let Some(to_label) = self.vertex_name_label_map.get(&to) {
                    let edge = Edge::new(
                        from,
                        to,
                        f_vertex.label.clone(),
                        to_label.to_string(),
                        e_label,
                    );
                    f_vertex.push(edge);
                    self.edge_size += 1;
                } else {
                    println!("Error: build_edge => {} 不存在 to_label.", to);
                }
            } else {
                println!("Error: build_edge => {} 不存在 vertex.", from);
            }
        }
    }
}

impl Graph {
    pub fn find_vertex(&self, name: &str) -> Option<&Vertex> {
        self.vertices.iter().find(|x| x.name == name)
    }

    fn push_node(&mut self, node: &Node) {
        let vertex = Vertex::from(node);
        self.vertex_name_label_map
            .insert(vertex.name.clone(), vertex.label.clone());
        self.vertices.push(vertex);
    }

    #[allow(dead_code)]
    fn build_edge_by_node(&mut self, node: &Node) {
        let to = &node.name;

        if let Some(to_label) = self.vertex_name_label_map.get(to) {
            for from_name in &node.input {
                if let Some(from) = self.vertices.iter_mut().find(|x| &x.name == from_name) {
                    let edge = Edge::new(
                        from_name.clone(),
                        to.to_string(),
                        from.label.clone(),
                        to_label.to_string(),
                        None,
                    );
                    from.push(edge);
                    self.edge_size += 1;
                } else {
                    // do nothing
                    // println!("WARN: build_edge => {} 不存在 vertex.", from_name);
                }
            }
        }
    }

    fn build_edges_for_nodes(&mut self, edges: Vec<(String, String)>) {
        for (from, to) in edges {
            if let Some(f_vertex) = self.vertices.iter_mut().find(|x| x.name == from) {
                if let Some(to_label) = self.vertex_name_label_map.get(&to) {
                    let edge = Edge::new(
                        from,
                        to,
                        f_vertex.label.clone(),
                        to_label.to_string(),
                        Some(Edge::NIL_E_LABEL.to_string()),
                    );
                    f_vertex.push(edge);
                    self.edge_size += 1;
                }
            }
        }
    }
}

impl Graph {
    pub fn graph_from_file(
        filename: &str,
        directed: bool,
    ) -> Result<Graph, GraphSetParseError> {
        //读取文件内容
        match fs::read_to_string(filename) {
            Ok(json_content) => {
                return Graph::graph_from_json_string(json_content, directed);
            }
            Err(_) => {
                return Err(GraphSetParseError {
                    message: format!("Error reading file : {}", filename),
                });
            }
        }
    }

    pub fn graph_from_json_string(
        json_content: String,
        directed: bool,
    ) -> Result<Graph, GraphSetParseError> {
        match serde_json::from_str::<ModelGraph>(&json_content) {
            Ok(model_graph) => {
                return Ok(Graph::graph_from_model_graph(model_graph, directed));
            }
            Err(e) => {
                return Err(GraphSetParseError {
                    message: format!("Error parsing json : {}", e.to_string()),
                });
            }
        }
    }

    pub fn graph_from_model_graph(model_graph: ModelGraph, directed: bool,) -> Graph {
        let node_map = model_graph.nodes;
        let mut graph = Graph::new(0, directed);
        graph.name = model_graph.name;
        for (_, val) in &node_map {
            graph.push_node(val);
        }
        // for (_, val) in &node_map {
        //     graph.build_edge_by_node(val);
        // }
        graph.build_edges_for_nodes(model_graph.edges);
        return graph;
    }

    pub fn to_str_repr(&self, support: Option<(usize, usize, usize, usize)>) -> String {
        let mut lines: Vec<String> = Vec::new();
        let mut g_rep = format!("t # {}", self.id.to_string());
        if let Some(support) = support {
            g_rep += &*format!(
                " * btw({}) inn({}, {}) ttl({})",
                support.0, support.1, support.2, support.3
            );
        }
        lines.push(g_rep);
        let mut edges: Vec<&Edge> = Vec::new();
        for vertex in &self.vertices {
            lines.push(vertex.to_str_repr());
            edges.extend(vertex.edges.iter());
        }
        for edge in edges {
            lines.push(edge.to_str_repr());
        }
        lines.join("\n")
    }
}

#[cfg(test)]
mod tests {
    use std::collections::BTreeMap;

    use super::*;

    #[test]
    fn test_load_single_graph() {
        let filename = r#"json\single-graph.json"#;

        match Graph::graph_from_file(&filename, true) {
            Ok(graph) => {
                println!("{}", graph.to_str_repr(None));
            }
            Err(e) => {
                println!("Error : {}", e)
            }
        }
    }

    #[test]
    fn test_load_graph() {
        let filename = r#"json\single-graph.json"#;

        match Graph::graph_from_file(&filename, true) {
            Ok(graph) => {
                // 使用flat_map展开内部有的集合，并收集到一个新的FxHashSet<String>
                let result: Vec<Edge> = graph
                    .vertices
                    .iter()
                    .map(|v| &v.edges)
                    .flat_map(|e| e.iter().cloned())
                    .collect();

                assert_eq!(9, result.len());

                let mut map: BTreeMap<String, (Vec<(&str, &str)>, usize)> = BTreeMap::new();

                for edge in result.iter() {
                    let key = format!(
                        "{}->{}->{}",
                        &edge.from_label, &edge.e_label, &edge.to_label
                    );
                    map.entry(key)
                        .and_modify(|v| {
                            v.0.push((&edge.from, &edge.to));
                            v.1 += 1
                        })
                        .or_insert((vec![(&edge.from, &edge.to)], 1));
                }

                print!("{:?}", map)
            }
            Err(e) => {
                println!("Error : {}", e)
            }
        }
    }
}
