use ahash::{HashMap, HashMapExt};

use crate::{Graph, Key};

impl Graph {
    pub(crate) fn add_constraints(&self, cg: &mut Graph, keys: &[Key]) {
        let mut prev_map: HashMap<Key, Key> = HashMap::new();
        let mut prev_root: Option<Key> = None;

        for key in keys {
            let mut current = self.parent(key);

            while let Some(child) = current {
                match self.parent(&child) {
                    Some(parent) => {
                        if let Some(&prev_child) = prev_map.get(&parent) {
                            if prev_child != child {
                                cg.set_edge(prev_child, child, None);
                                return;
                            }
                        }
                        prev_map.insert(parent, child);
                    }
                    None => {
                        if let Some(prev_child) = prev_root {
                            if prev_child != child {
                                cg.set_edge(prev_child, child, None);
                                return;
                            }
                        }
                        prev_root = Some(child);
                    }
                }
                current = self.parent(&child);
            }
        }
    }
}
