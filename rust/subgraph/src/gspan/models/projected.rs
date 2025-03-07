/*
 * Copyright (c), Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
use rustc_hash::FxHashSet;

use crate::gspan::models::{edge::Edge, prev_dfs::PrevDFS};
// PrevDFS 链表节点的集合
// 链表节点可以表示为一个子图，因此 Projected 相当于子图的集合
// projections 前后的
/**
 * Projected 最主要的作用是在栈中保存所有的 projections ，它是一个 PrevDFS 的数组。
 * 在递归调用的子挖掘的搜索栈中，每次传入一个 Projected ，代表当前的 DFSCode 在所有原图中的“投影” （出现位置及每条边被添加的顺序），
 */
#[derive(Debug)]
pub struct Projected<'a> {
    pub projections: Vec<Box<PrevDFS<'a>>>,
}

impl<'a> Projected<'a> {
    pub fn new() -> Projected<'a> {
        Projected { projections: Vec::with_capacity(32) }
    }

    pub fn push(&mut self, id: usize, edge: &'a Edge, prev: Option<&'a PrevDFS<'a>>) {
        let new_pdfs = PrevDFS::new(id, edge, prev);
        self.projections.push(Box::new(new_pdfs));
    }

    pub fn to_vertex_names_list(&self) -> Vec<FxHashSet<(usize, String)>> {
        self.projections.iter().map(|p| p.get_vertex_names()).collect()
    }

    pub fn to_edges_list(&self) -> Vec<Vec<&Edge>> {
        self.projections.iter().map(|p| p.get_edges()).collect()
    }
}
