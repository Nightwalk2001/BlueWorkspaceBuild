/*
 * Copyright (c), Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
use std::collections::HashSet;

use super::projected::Projected;
use crate::gspan::{
    models::{dfs::DFS, graph::Graph},
    result::MaxDFSCodeGraphResult,
};

/**
 * DFSCode 的本质是图中所有边的信息的排列，每条边上记录顶点 ID、顶点和边的Label。
 * 该排列具有某种全序关系定义，因此在该全序关系上具有最小值，我们可只对具有最小值的 DFSCode 进行拓展挖掘。
 * 图同构等价于 DFSCode 相同。
 */
#[derive(Debug, Clone)]
pub struct DFSCode {
    is_push_result: bool,
    pub dfs_vec: Vec<DFS>,
}

impl DFSCode {
    pub fn new() -> DFSCode {
        DFSCode { is_push_result: false, dfs_vec: Vec::with_capacity(32) }
    }

    pub fn push(
        &mut self,
        from: usize,
        to: usize,
        from_label: String,
        e_label: String,
        to_label: String,
    ) {
        self.is_push_result = false;
        self.dfs_vec.push(DFS::from(from, to, from_label, e_label, to_label));
    }

    pub fn pop_with_set_result(
        &mut self,
        projected: &Projected,
        result: &mut MaxDFSCodeGraphResult,
    ) -> Option<DFS> {
        if !self.is_push_result {
            // 记录尽可能远的深度搜索的结果
            // println!("pop {} {} {:?}", singleton.min_sup, singleton.inner_min_sup, singleton.out);
            self.is_push_result = result.add_value(self, projected);
        }
        return self.dfs_vec.pop();
    }

    pub fn get_dfs(&self, index: usize) -> &DFS {
        self.dfs_vec.get(index).unwrap()
    }

    pub fn to_graph(&self, graph_id: usize, directed: bool) -> Graph {
        let mut g = Graph::new(graph_id, directed);
        let mut edge_data = Vec::<(String, String, Option<String>)>::with_capacity(8);
        for it in &self.dfs_vec {
            let from_name = it.from.to_string();
            if it.from_label != "" && !g.vertex_name_label_map.contains_key(&from_name) {
                g.insert_vertex(&from_name, &it.from_label);
            }
            let to_name = it.to.to_string();
            if it.to_label != "" && !g.vertex_name_label_map.contains_key(&to_name) {
                g.insert_vertex(&to_name, &it.to_label);
            }

            // build_edge
            edge_data.push((from_name, to_name, Some(it.e_label.clone())));
        }
        g.build_edge(edge_data);
        g
    }

    // 返回DFS过程中从最尾到根的边的序列索引，作为深度搜索逆路径
    // 其中要求 from -> to 必须是字典序大小排列
    pub fn build_rm_path(&self) -> Vec<usize> {
        let mut rm_path: Vec<usize> = Vec::new();
        let mut old_from = usize::MAX;
        for i in (0..self.dfs_vec.len()).rev() {
            let dfs = self.dfs_vec.get(i).unwrap();
            if dfs.from < dfs.to && (rm_path.is_empty() || old_from == dfs.to) {
                rm_path.push(i);
                old_from = dfs.from;
            }
        }
        rm_path
    }

    pub fn count_node(&self) -> usize {
        let mut count = HashSet::new();
        for dfs in &self.dfs_vec {
            count.insert(&dfs.from);
            count.insert(&dfs.to);
        }
        count.len()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // 辅助函数用于创建 Edge 并添加到 DFSCode 中
    fn create_and_add_edges(dfs_code: &mut DFSCode) {
        dfs_code.push(1, 2, String::from("a"), String::from("A"), String::from("b"));
        dfs_code.push(2, 3, String::from("b"), String::from("A"), String::from("c"));
        dfs_code.push(2, 1, String::from("b"), String::from("B"), String::from("a"));
    }

    #[test]
    fn test_to_graph() {
        let mut dfs_code = DFSCode::new();
        create_and_add_edges(&mut dfs_code);

        let g = dfs_code.to_graph(0, true);
        assert_eq!(
            r#"t # 0
v 1 a
v 2 b
v 3 c
e 1 2 a b A
e 2 3 b c A
e 2 1 b a B"#,
            g.to_str_repr(None)
        );
    }

    #[test]
    fn test_rm_path() {
        let mut dfs_code = DFSCode::new();
        create_and_add_edges(&mut dfs_code);

        let rm_path_str = dfs_code
            .build_rm_path()
            .iter()
            .map(|n| n.to_string())
            .collect::<Vec<String>>()
            .join(",");

        // 使用有意义的错误消息
        assert_eq!("1,0", rm_path_str, "Expected RMPath to be '1,0', but found '{}'", rm_path_str);
    }
}
