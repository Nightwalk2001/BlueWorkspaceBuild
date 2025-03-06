use ahash::{HashMap, HashMapExt};

use crate::{Graph, Key, KeyCodecExt, EMPTY_KEY};

#[derive(Default)]
pub struct BlockGraph {
    nodes: HashMap<Key, Key>,
    pub(super) in_edges: HashMap<Key, Vec<Key>>,
    predecessors: HashMap<Key, HashMap<Key, usize>>,
    pub(super) edges: HashMap<Key, f32>,
}

impl BlockGraph {
    pub fn set_node(&mut self, key: Key) -> &mut Self {
        if self.nodes.contains_key(&key) {
            return self;
        }

        self.nodes.insert(key, EMPTY_KEY);
        self.in_edges.insert(key, vec![]);
        self.predecessors.insert(key, HashMap::new());

        self
    }

    #[inline]
    pub fn nodes(&self) -> Vec<Key> {
        self.nodes.keys().copied().collect()
    }

    #[inline]
    pub fn predecessors(&self, k: &Key) -> Vec<Key> {
        self.predecessors.get(k).map_or(vec![], |p| p.keys().copied().collect())
    }

    pub fn set_edge(&mut self, source: Key, target: Key, val: f32) -> &mut Self {
        let key = Key::of(source, target);
        if self.edges.contains_key(&key) {
            self.edges.insert(key, val);
            return self;
        }

        self.set_node(source);
        self.set_node(target);

        self.edges.insert(key, val);

        self.predecessors
            .get_mut(&target)
            .map(|preds| preds.entry(source).and_modify(|c| *c += 1).or_insert(1));

        self.in_edges.entry(target).or_default().push(key);

        self
    }
}

impl Graph {
    pub(super) fn build_block_graph(
        &self,
        matrix: &[Vec<Key>],
        root: &HashMap<Key, Key>,
    ) -> BlockGraph {
        let mut block_graph: BlockGraph = BlockGraph::default();

        for keys in matrix {
            let mut target: Option<Key> = None;
            for &key in keys {
                let source = root[&key];
                block_graph.set_node(source);
                if let Some(t) = target {
                    let target = root[&t];
                    let prev_max = match block_graph.edges.get(&Key::of(target, source)) {
                        Some(&x) => x,
                        None => 0.0,
                    };

                    let max = self.sep(key, t).max(prev_max);
                    block_graph.set_edge(target, source, max);
                }
                target = Some(key);
            }
        }

        block_graph
    }

    fn sep(&self, source: Key, target: Key) -> f32 {
        let nodesep = self.config.nodesep;
        let edgesep = self.config.edgesep;

        let source_node = &self.nodes[&source];
        let target_node = &self.nodes[&target];

        let mut sum = source_node.width / 2.0;
        sum += if source_node.dummy.is_some() { edgesep } else { nodesep } / 2.0;
        sum += if target_node.dummy.is_some() { edgesep } else { nodesep } / 2.0;
        sum += target_node.width / 2.0;

        sum
    }
}
