use std::mem;

use ahash::{HashSet, HashSetExt};

use crate::{Edge, Graph, GraphEdge, GraphNode, Key, EMPTY_KEY};

impl Graph {
    pub(super) fn network_simplex(&mut self) {
        self.simplify_ref();
        self.longest_path();

        let mut t: Graph = self.feasible_tree();
        init_low_lim_values(&mut t);
        init_cut_values(&mut t, self);

        while let Some(e) = leave_edge(&t) {
            if let Some(f) = enter_edge(&t, &self, e) {
                exchange_edges(&mut t, self, e, f);
            }
        }
    }

    fn simplify_ref(&mut self) {
        for edge in self.edge_values.values_mut() {
            edge.weight.get_or_insert(0.0);
            edge.minlen.get_or_insert(1);
        }
    }
}

fn init_cut_values(t: &mut Graph, g: &mut Graph) {
    let keys = g.postorder(&g.nodes());
    for &key in keys.iter().skip(1) {
        assign_cut_value(t, g, key);
    }
}

fn assign_cut_value(t: &mut Graph, g: &mut Graph, child: Key) {
    let cutvalue = calc_cut_value(t, g, child);
    if let Some(node) = t.node_mut(&child) {
        let parent = node.parent.unwrap_or(EMPTY_KEY);
        if let Some(edge) = t.edge_mut(child, parent) {
            edge.cutvalue = Some(cutvalue);
        }
    }
}

fn calc_cut_value(t: &mut Graph, g: &mut Graph, child: Key) -> f32 {
    let Some(node) = t.node_mut(&child) else { return 0.0 };

    let parent = node.parent.unwrap_or(EMPTY_KEY);
    let mut child_is_tail = true;
    let mut graph_edge = g.edge_mut(child, parent);

    if graph_edge.is_none() {
        child_is_tail = false;
        graph_edge = g.edge_mut(parent, child);
    }

    let mut cut_value = graph_edge.and_then(|e| e.weight).unwrap_or(0.0);

    for edge in g.node_edges(&child) {
        let is_out_edge = edge.source == child;
        let other = if is_out_edge { edge.target } else { edge.source };

        if other == parent {
            continue;
        }

        let points_to_head = is_out_edge == child_is_tail;
        let other_weight = g.edge1(edge).and_then(|e| e.weight).unwrap_or(0.0);

        cut_value += if points_to_head { other_weight } else { -other_weight };

        if is_tree_edge(t, child, other) {
            let out_cut_value = t.edge(child, other).and_then(|e| e.cutvalue).unwrap_or(0.0);
            cut_value += if points_to_head { -out_cut_value } else { out_cut_value }
        }
    }

    cut_value
}

fn init_low_lim_values(tree: &mut Graph) {
    let root = tree.nodes().first().copied().unwrap_or(EMPTY_KEY);
    let mut visited: HashSet<Key> = HashSet::new();
    assign_low_lim(tree, &mut visited, 1, root);
}

fn assign_low_lim(
    tree: &mut Graph,
    visited: &mut HashSet<Key>,
    mut next_lim: usize,
    start_key: Key,
) -> usize {
    let mut stack: Vec<(Key, usize, Option<Key>)> = vec![(start_key, next_lim, None)];

    while let Some((k, low, parent)) = stack.pop() {
        if !visited.insert(k) {
            continue;
        }

        let neighbors = tree.neighbors(&k);
        let unvisited_neighbors =
            neighbors.into_iter().filter(|w| !visited.contains(w)).collect::<Vec<_>>();

        if !unvisited_neighbors.is_empty() {
            stack.push((k, low, parent));

            for t in unvisited_neighbors {
                stack.push((t, next_lim, Some(k)));
            }
        } else {
            if let Some(node) = tree.node_mut(&k) {
                node.low = Some(low);
                node.lim = Some(next_lim);
                next_lim += 1;

                node.parent = parent;
            }
        }
    }

    next_lim
}

fn leave_edge(tree: &Graph) -> Option<Edge> {
    tree.edges
        .values()
        .find(|&&edge_obj| tree.edge1(edge_obj).map(|e| e.cutvalue) < Some(Some(0.0)))
        .copied()
}

fn enter_edge(t: &Graph, g: &Graph, edge: Edge) -> Option<Edge> {
    let mut source = edge.source;
    let mut target = edge.target;

    if !g.has_edge(source, target) {
        mem::swap(&mut source, &mut target);
    }

    let source_node = t.node(&source);
    let target_node = t.node(&target);
    let mut tail_node = source_node;
    let mut flip = false;

    if source_node?.lim > target_node?.lim {
        tail_node = target_node;
        flip = true;
    }

    g.edges
        .values()
        .filter(|edge_obj| {
            let v_node = t.node(&edge_obj.source);
            let w_node = t.node(&edge_obj.target);
            flip == is_descendant(v_node, tail_node) && flip != is_descendant(w_node, tail_node)
        })
        .min_by_key(|&&e| g.slack(e))
        .copied()
}

fn exchange_edges(t: &mut Graph, g: &mut Graph, e: Edge, f: Edge) {
    t.remove_edge(e.source, e.target);
    t.set_edge(f.source, f.target, Some(GraphEdge::default()));
    init_low_lim_values(t);
    init_cut_values(t, g);
    update_ranks(t, g);
}

fn update_ranks(t: &mut Graph, g: &mut Graph) {
    let root = t
        .nodes
        .keys()
        .find(|k| !g.node(k).map_or(true, |n| n.parent.is_none()))
        .copied()
        .unwrap_or(EMPTY_KEY);
    let keys = t.preorder(&vec![root]);
    for &k in keys.iter().skip(1) {
        let parent = t.node(&k).and_then(|n| n.parent).unwrap_or(EMPTY_KEY);
        let mut edge = g.edge(k, parent);
        let mut flipped = false;
        if edge.is_none() {
            edge = g.edge(parent, k);
            flipped = true;
        }

        let mut minlen = edge.and_then(|e| e.minlen).unwrap_or(0);
        if !flipped {
            minlen = -minlen
        }

        let parent_rank = g.node(&parent).and_then(|n| n.rank).unwrap_or(0);
        if let Some(node) = g.node_mut(&k) {
            node.rank = Some(parent_rank + (minlen));
        }
    }
}

#[inline]
fn is_tree_edge(tree: &Graph, source: Key, target: Key) -> bool {
    tree.has_edge(source, target)
}

fn is_descendant(node: Option<&GraphNode>, root_node: Option<&GraphNode>) -> bool {
    let root_node = root_node.unwrap();
    let lim = node.unwrap().lim;
    root_node.low <= lim && lim <= root_node.lim
}
