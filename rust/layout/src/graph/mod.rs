mod config;
mod key;
mod node_edge;

use std::{cmp::PartialEq, fmt::Debug};

use ahash::{HashMap, HashMapExt};
pub use config::*;
pub use key::*;
pub use node_edge::*;

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub struct Edge {
    pub source: Key,
    pub target: Key,
}

impl Edge {
    #[inline]
    pub fn of(source: Key, target: Key) -> Self {
        Self { source, target }
    }

    #[inline]
    pub fn to_key(self) -> Key {
        Key::of(self.source, self.target)
    }
}

/// ### Compiler
/// Currently all hashMaps are stored in this structure.
///
/// It's challenging to avoid
/// **can't borrow as mutable while borrowed as immutable**.
///
/// ### Performance
/// Currently, almost all data is stored in [`HashMap`].
/// There may be a better solution.
/// Node's Key is usize, which is naturally an array index.
/// If store all Nodes in Vec,
/// which can achieve extremely high performance.
///
/// At the same time, there are some points to note:
///
/// 1. Vec should store [`Option<Node>`] instead of Node,
/// or add an [`is_deleted`] field to Node,
/// so that better cache affinity and performance can be obtained,
/// and the code is more concise.
///
/// 2. Under the premise of the first point,
/// we can convert all insert operations into push,
/// so that there's no [`Memory-Move`].
///
/// 3. Implement [`Indexing`] is needed
/// while the expansion of 10w+ Nodes is a disaster.
/// In our scenario, only one level of indexing is needed.
/// The maximum capacity of each array can be 1024.
/// Use [`LinkedList`] to store these arrays.
///
/// 4. After implementing [`Indexing`]
/// almost all operations have a [`branch-judgment`].
/// We should think of some ways to eliminate this performance consumption.
/// Cumbersome and repetitive code is also acceptable.
/// Or use [`likely-unlikely`] to prompt the compiler
#[derive(Debug, Default)]
pub struct Graph {
    pub is_directed: bool,
    pub is_compound: bool,
    pub config: GraphConfig,
    pub width: f32,
    pub height: f32,
    pub nodes: HashMap<Key, GraphNode>,
    pub in_map: HashMap<Key, Vec<Edge>>,
    pub predecessors: HashMap<Key, HashMap<Key, usize>>,
    pub out_map: HashMap<Key, Vec<Edge>>,
    pub successors: HashMap<Key, HashMap<Key, usize>>,
    pub edges: HashMap<Key, Edge>,
    pub edge_values: HashMap<Key, GraphEdge>,
    pub parent_map: HashMap<Key, Key>,
    pub children_map: HashMap<Key, Vec<Key>>,
    pub selfedge_map: HashMap<Key, Vec<Edge>>,
    pub nesting_root: Option<Key>,
    pub root: Option<Key>,
    pub dummy_chains: Option<Vec<Key>>,
}

impl Graph {
    #[inline]
    fn of() -> Self {
        Self { is_directed: true, ..Self::default() }
    }
}

impl Graph {
    #[inline]
    pub fn new(directed: bool, compound: bool) -> Self {
        let mut graph = Self::of();

        graph.is_directed = directed;
        graph.is_compound = compound;

        if compound {
            graph.parent_map = HashMap::new();
            graph.children_map = HashMap::new();
            graph.children_map.insert(EMPTY_ROOT, vec![]);
        }

        graph
    }

    #[inline]
    pub fn nodes(&self) -> Vec<Key> {
        self.nodes.keys().copied().collect()
    }

    #[inline]
    pub fn sources(&self) -> Vec<Key> {
        self.nodes
            .keys()
            .filter(|&n| self.in_map.get(n).map_or(true, |m| m.is_empty()))
            .copied()
            .collect()
    }

    #[inline]
    pub fn sinks(&self) -> Vec<Key> {
        self.nodes
            .keys()
            .filter(|&n| self.out_map.get(n).map_or(true, |m| m.is_empty()))
            .copied()
            .collect()
    }

    #[inline]
    pub fn set_node(&mut self, key: Key, value: Option<GraphNode>) -> &mut Self {
        if self.nodes.contains_key(&key) {
            value.map(|new_node| self.nodes.insert(key, new_node));
            return self;
        }

        self.nodes.insert(key, value.unwrap_or_default());

        if self.is_compound {
            self.parent_map.insert(key, EMPTY_ROOT);
            self.children_map.insert(key, vec![]);
            self.children_map.entry(EMPTY_ROOT).or_default();
        }

        self.in_map.insert(key, vec![]);
        self.predecessors.insert(key, HashMap::new());
        self.out_map.insert(key, vec![]);
        self.successors.insert(key, HashMap::new());

        self
    }

    #[inline]
    pub fn node(&self, id: &Key) -> Option<&GraphNode> {
        self.nodes.get(id)
    }

    #[inline]
    pub fn node_mut(&mut self, id: &Key) -> Option<&mut GraphNode> {
        self.nodes.get_mut(id)
    }

    #[inline]
    pub fn has_node(&self, id: &Key) -> bool {
        self.nodes.contains_key(id)
    }

    pub fn remove_node(&mut self, id: &Key) {
        if let Some(_) = self.nodes.remove(id) {
            if self.is_compound {
                self.remove_from_parents_child_list(id);
                self.parent_map.remove(id);
                for child_id in self.children(id) {
                    self.set_parent(child_id, None);
                }
                self.children_map.remove(id);
            }

            self.in_map.remove(id).map(|in_edges| {
                for edge in in_edges {
                    self.remove_edge1(edge);
                }
            });

            self.predecessors.remove(id);

            self.out_map.remove(id).map(|out_edges| {
                for edge in out_edges {
                    self.remove_edge1(edge);
                }
            });

            self.successors.remove(id);
        }
    }

    pub fn set_parent(&mut self, id: Key, parent: Option<Key>) -> &mut Self {
        let ancestor = match parent {
            Some(p) => {
                let mut current = p;
                while let Some(new_ancestor) = self.parent(&current) {
                    current = new_ancestor;
                }
                current
            }
            None => EMPTY_ROOT,
        };

        self.set_node(id, None);
        self.remove_from_parents_child_list(&id);
        self.parent_map.insert(id, ancestor);
        self.children_map.entry(ancestor).or_default().push(id);

        self
    }

    #[inline]
    fn remove_from_parents_child_list(&mut self, id: &Key) {
        self.parent_map
            .get(id)
            .map(|p| self.children_map.get_mut(p).map(|c| c.retain(|c| c != id)));
    }

    #[inline]
    pub fn parent(&self, id: &Key) -> Option<Key> {
        self.is_compound
            .then(|| self.parent_map.get(id).filter(|&&p| p != EMPTY_ROOT).copied())?
    }

    pub fn children(&self, id: &Key) -> Vec<Key> {
        match (self.is_compound, id == &EMPTY_ROOT) {
            (true, _) => self
                .children_map
                .get(id)
                .map_or(vec![], |children| children.iter().copied().collect()),
            (false, true) => self.nodes.keys().copied().collect(),
            _ => vec![],
        }
    }

    #[inline]
    pub fn predecessors(&self, id: &Key) -> Vec<Key> {
        self.predecessors.get(id).unwrap().keys().copied().collect()
    }

    #[inline]
    pub fn successors(&self, id: &Key) -> Vec<Key> {
        self.successors.get(id).unwrap().keys().copied().collect()
    }

    #[inline]
    pub fn neighbors(&self, id: &Key) -> Vec<Key> {
        let mut ret = self.predecessors(id);
        ret.extend(self.successors(id));
        ret
    }

    #[inline]
    pub fn navigation(&self, id: &Key) -> Vec<Key> {
        if self.is_directed { self.successors(id) } else { self.neighbors(id) }
    }

    #[inline]
    pub fn edges(&self) -> Vec<Edge> {
        self.edges.values().copied().collect()
    }

    pub fn set_edge(&mut self, source: Key, target: Key, edge: Option<GraphEdge>) -> &mut Self {
        let key = Key::of(source, target);
        if self.edge_values.contains_key(&key) {
            if let Some(edge) = edge {
                self.edge_values.insert(key, edge);
            }
            return self;
        }

        self.set_node(source, None);
        self.set_node(target, None);

        if let Some(mut edge) = edge {
            edge.source = source;
            edge.target = target;
            self.edge_values.insert(key, edge);
        } else {
            self.edge_values.insert(key, GraphEdge::of(source, target));
        }

        let edge = Edge::of(source, target);

        self.edges.insert(key, edge);
        if let Some(preds) = self.predecessors.get_mut(&target) {
            preds.entry(source).and_modify(|c| *c += 1).or_insert(1);
        }
        if let Some(succ) = self.successors.get_mut(&source) {
            succ.entry(target).and_modify(|c| *c += 1).or_insert(1);
        }

        self.in_map.entry(target).or_default().push(edge);
        self.out_map.entry(source).or_default().push(edge);

        self
    }

    #[inline]
    pub fn set_edge_undirected(
        &mut self,
        source: Key,
        target: Key,
        edge: Option<GraphEdge>,
    ) -> &mut Self {
        let (source, target) = normalize_st(source, target);

        self.set_edge(source, target, edge)
    }

    #[inline]
    pub fn edge(&self, source: Key, target: Key) -> Option<&GraphEdge> {
        let key = Key::of(source, target);
        self.edge_values.get(&key)
    }

    #[inline]
    pub fn edge_mut(&mut self, source: Key, target: Key) -> Option<&mut GraphEdge> {
        let key = Key::of(source, target);
        self.edge_values.get_mut(&key)
    }

    #[inline]
    pub fn has_edge(&self, source: Key, target: Key) -> bool {
        let key = Key::of(source, target);
        self.edge_values.contains_key(&key)
    }

    pub fn remove_edge(&mut self, source: Key, target: Key) -> &mut Self {
        let key = Key::of(source, target);

        if let Some(edge) = self.edges.get(&key) {
            let s = &edge.source;
            let t = &edge.target;

            if let Some(in_edges) = self.in_map.get_mut(t) {
                in_edges.retain(|e| e != edge)
            }
            if let Some(out_edges) = self.out_map.get_mut(s) {
                out_edges.retain(|e| e != edge)
            }

            if let Some(pred) = self.predecessors.get_mut(t) {
                decrement_or_remove(pred, &s)
            }

            if let Some(suc) = self.successors.get_mut(s) {
                decrement_or_remove(suc, &t)
            }

            self.edge_values.remove(&key);
            self.edges.remove(&key);
        }

        self
    }

    #[inline]
    pub fn edge1(&self, edge: Edge) -> Option<&GraphEdge> {
        self.edge(edge.source, edge.target)
    }

    #[inline]
    pub fn edge_mut1(&mut self, edge: Edge) -> Option<&mut GraphEdge> {
        self.edge_mut(edge.source, edge.target)
    }

    #[inline]
    pub fn set_edge1(
        &mut self,
        Edge { source, target }: Edge,
        edge: Option<GraphEdge>,
    ) -> &mut Self {
        self.set_edge(source, target, edge)
    }

    #[inline]
    pub fn remove_edge1(&mut self, edge: Edge) -> &mut Self {
        self.remove_edge(edge.source, edge.target)
    }

    #[inline]
    pub fn in_edges(&self, key: &Key) -> Vec<Edge> {
        self.in_map[key].clone()
    }

    #[inline]
    pub fn out_edges(&self, key: &Key) -> Vec<Edge> {
        self.out_map[key].clone()
    }

    #[inline]
    pub fn node_edges(&self, key: &Key) -> Vec<Edge> {
        let mut ret = self.in_edges(key);
        ret.extend(self.out_edges(key));
        ret
    }
}

#[inline]
fn decrement_or_remove(map: &mut HashMap<Key, usize>, k: &Key) {
    if let Some(value) = map.get_mut(k) {
        *value -= 1;
        if *value <= 0 {
            map.remove(k);
        }
    }
}
