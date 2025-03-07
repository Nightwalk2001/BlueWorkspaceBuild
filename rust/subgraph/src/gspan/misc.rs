/*
 * Copyright (c), Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
use std::collections::{BTreeMap, HashMap};

use rustc_hash::FxHashSet;

use super::models::dfs_code::DFSCode;
use crate::gspan::models::{
    edge::Edge, graph::Graph, history::History, projected::Projected, vertex::Vertex,
};

// 计算某个子图模式在图集合中的支持度
// 计算 projected 内的 PrevDFS 的实值个数，即 projected 中所表示的子图模式出现过的不同的图的数量
pub fn support(projected: &Projected) -> usize {
    let mut oid = usize::MAX;
    let mut size = 0;

    for cur in projected.projections.iter() {
        // 如果当前元素的图ID（cur.gid）与前一个元素的图ID（oid）不同，说明遇到了一个新的图，因此增加计数器。
        if oid != cur.gid {
            size += 1;
        }
        oid = cur.gid;
    }
    size
}

// 计算某个子图模式在单个图中的支持度
// 计算 projected 内的 PrevDFS 的实值个数，即 projected 中所表示的子图模式出现过的相同图的数量
// CORE：特殊处理，要去掉节点完全相同的投影情况
pub fn inner_support(projected: &Projected) -> (usize, usize) {
    let mut count_map = HashMap::<usize, usize>::new();

    let mut unify_vertices_list: Vec<FxHashSet<String>> = vec![];
    for cur in projected.projections.iter() {
        let set: FxHashSet<String> =
            cur.get_vertex_names().iter().map(|f| format!("{}/{}", &f.0, &f.1)).collect();
        // 如果存在
        if unify_vertices_list.contains(&set) {
            continue;
        }

        unify_vertices_list.push(set);
        count_map.entry(cur.gid).and_modify(|v| *v += 1).or_insert(1);
    }

    let mut min = usize::MAX;
    let mut max = usize::MIN;
    for (_, v) in count_map.iter() {
        min = if min > *v { *v } else { min };
        max = if max < *v { *v } else { max };
    }

    (min, max)
}

// 获取图中某节点引出的有效边（from.label <= to.label）, 用于构造 DFSCode
pub fn get_forward_edges<'a>(g: &Graph, v: &'a Vertex) -> Vec<&'a Edge> {
    let mut result: Vec<&Edge> = Vec::with_capacity(8);
    for edge in &v.edges {
        if v.label <= g.vertex_name_label_map.get(&edge.to).unwrap().to_string() {
            result.push(edge);
        }
    }
    result
}

// 获取图中最右下节点到最右路径上的Backward边
// 获取 e2 的终点出发的边 edge，这个边的终点是 e1 的起点
// 且满足 edge 的标签大于 e1 的标签
pub fn get_backward<'a, 'b>(
    g: &'a Graph,
    e1: &'a Edge,
    e2: &'a Edge,
    history: &'b History,
) -> Option<&'a Edge> {
    if e1 == e2 {
        return None;
    }
    // 遍历从e2的终点出发的所有边
    for edge in &g.find_vertex(&e2.to).unwrap().edges {
        if history.has_edge(&edge.id) || edge.to != e1.from {
            continue;
        }
        // 找到一个边的终点是e1的起点
        if e1.e_label < edge.e_label
            || (e1.e_label == edge.e_label
                && g.vertex_name_label_map.get(&e1.to).unwrap()
                    <= g.vertex_name_label_map.get(&e2.to).unwrap())
        {
            return Some(&edge);
        }
    }
    return None;
}

// 获取图中最右下节点引出的所有 Forward 边
pub fn get_forward_pure<'a, 'b>(
    g: &'a Graph,
    e: &'b Edge,
    min_label: &str,
    history: &'b History,
) -> Vec<&'a Edge> {
    let mut result: Vec<&Edge> = Vec::with_capacity(8);
    for edge in &g.find_vertex(&e.to).unwrap().edges {
        if min_label > g.vertex_name_label_map.get(&edge.to).unwrap().as_str()
            || history.has_vertex(&edge.to)
        {
            continue;
        }
        result.push(&edge);
    }
    result
}

// 获取图中最右路路径引出的所有 Forward 边
pub fn get_forward_rm_path<'a, 'b>(
    g: &'a Graph,
    e: &'b Edge,
    min_label: &str,
    history: &'b History,
) -> Vec<&'a Edge> {
    let mut result: Vec<&Edge> = Vec::with_capacity(8);
    let to_label = g.vertex_name_label_map.get(&e.to).unwrap().as_str();

    for edge in &g.find_vertex(&e.from).unwrap().edges {
        let to_label_2 = g.vertex_name_label_map.get(&edge.to).unwrap().as_str();
        if e.to == edge.to || min_label > to_label_2 || history.has_vertex(&edge.to) {
            continue;
        }
        if e.e_label < edge.e_label || (e.e_label == edge.e_label && to_label <= to_label_2) {
            result.push(&edge);
        }
    }
    result
}
