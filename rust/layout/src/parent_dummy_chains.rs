use ahash::{HashMap, HashMapExt};

use crate::{Edge, Graph, Key, EMPTY_KEY, EMPTY_ROOT};

impl Graph {
    /// Processes dummy chains in the graph
    /// to establish proper parent-child relationships.
    ///
    /// This method performs a post-order traversal to determine node limits,
    /// then processes each
    /// dummy chain to find the lowest common ancestor
    /// (LCA) and adjust parent relationships along
    /// the path between edge endpoints.
    ///
    /// # Returns
    /// - `Option<()>`: always returns None,
    pub(super) fn parent_dummy_chains(&mut self) -> Option<()> {
        let lims = postorder(self);
        let dummy_chains = self.dummy_chains.clone().unwrap_or(vec![]);

        for mut dummy_id in dummy_chains {
            let edge = self.node(&dummy_id)?.edge?;
            let (path, ref lca) = self.find_lca(&lims, edge.source, edge.target);

            if !path.is_empty() {
                self.traverse_path(&mut dummy_id, &path, lca, edge)?;
            }
        }

        None
    }

    /// Traverses a path between nodes
    /// while setting parent relationships for dummy nodes.
    ///
    /// # Arguments
    /// * `dummy_id` - Mutable reference to the current dummy node key
    /// * `path` - Path through which to establish relationships
    /// * `lca` - Lowest common ancestor of the edge endpoints
    /// * `edge` - Original edge being processed
    fn traverse_path(
        &mut self,
        dummy_id: &mut Key,
        path: &[Key],
        lca: &Key,
        edge: Edge,
    ) -> Option<()> {
        let mut path_iter = path.iter().peekable();
        let mut ascending = true;

        while *dummy_id != edge.target {
            let node = self.node(dummy_id)?;
            let mut current = (if ascending {
                path_iter.find(|k| self.node(k).unwrap().max_rank > node.rank)
            } else {
                path_iter.rfind(|k| self.node(k).unwrap().min_rank < node.rank)
            })
                .unwrap_or(lca);

            if ascending && current == lca {
                ascending = false;
                path_iter = path.iter().peekable();
                current = path_iter.next_back()?;
            }

            self.set_parent(*dummy_id, Some(*current));
            *dummy_id = self.successors(dummy_id).first().copied().unwrap_or(EMPTY_KEY)
        }

        None
    }


    /// Finds the lowest common ancestor (LCA) and constructs a path between two nodes.
    ///
    /// # Arguments
    /// * `lims` - Post-order traversal limits map
    /// * `source` - Source node key
    /// * `target` - Target node key
    ///
    /// # Returns
    /// - `(Vec<Key>, Key)`: tuple containing:
    ///   - Full path from source to target through LCA
    ///   - Lowest common ancestor key
    fn find_lca(&self, lims: &HashMap<Key, i32>, source: Key, target: Key) -> (Vec<Key>, Key) {
        let mut s_path: Vec<Key> = vec![];
        let mut t_path: Vec<Key> = vec![];

        let lim = lims[&source].min(lims[&target]);

        let mut lca = source;
        while let Some(parent) = self.parent(&lca) {
            lca = parent;
            s_path.push(parent);
            if lims.get(&parent).map_or(false, |&l| lim == l) {
                break;
            }
        }

        let mut parent = self.parent(&target).unwrap_or(lca);
        while parent != lca {
            t_path.push(parent);
            parent = self.parent(&parent).unwrap_or(lca);
        }

        t_path.reverse();
        s_path.extend(t_path);

        (s_path, lca)
    }
}

/// Generates post-order traversal limits for graph nodes.
fn postorder(g: &Graph) -> HashMap<Key, i32> {
    let mut ret: HashMap<Key, i32> = HashMap::new();
    let mut lim = 0;

    for child in g.children(&EMPTY_ROOT) {
        let mut stack = vec![child];

        while let Some(node) = stack.pop() {
            if ret.contains_key(&node) {
                continue;
            }

            ret.insert(node, lim);
            lim += 1;
            stack.extend(g.children(&node))
        }
    }

    ret
}
