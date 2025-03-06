use ahash::{HashSet, HashSetExt};

use crate::{Graph, Key};

impl Graph {
    #[inline]
    pub(super) fn preorder(&self, keys: &[Key]) -> Vec<Key> {
        self.traverse(keys, false)
    }

    #[inline]
    pub(super) fn postorder(&self, keys: &[Key]) -> Vec<Key> {
        self.traverse(keys, true)
    }

    fn traverse(&self, keys: &[Key], postorder: bool) -> Vec<Key> {
        let mut acc: Vec<Key> = Vec::with_capacity(keys.len() * 2);
        let mut visited: HashSet<Key> = HashSet::new();
        let mut stack = vec![];

        for &key in keys {
            if visited.contains(&key) {
                continue;
            }
            stack.push(key);

            while let Some(curr) = stack.pop() {
                if !visited.contains(&curr) {
                    visited.insert(curr);
                    if !postorder {
                        acc.push(curr);
                    }

                    let mut neighbors = self.navigation(&curr);
                    if !postorder {
                        neighbors.reverse();
                    }
                    stack.extend(neighbors);
                }
            }
        }

        if postorder {
            acc.reverse();
        }

        acc
    }
}
