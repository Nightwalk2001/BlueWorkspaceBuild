/*
 * Copyright (c), Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

// DFS ∈ Edge， 是没有 edge id 的五元组
// 但要注意的是，DFS 上的 from to 和 Edge 上的 from to 无关
// DFS 是子图的五元组，Edge 是完整图的五元组
#[derive(PartialEq, Debug, Clone)]
pub struct DFS {
    pub from: usize,
    pub to: usize,
    pub from_label: String,
    pub e_label: String,
    pub to_label: String,
}

impl DFS {
    pub fn from(
        from: usize,
        to: usize,
        from_label: String,
        e_label: String,
        to_label: String,
    ) -> DFS {
        DFS { from, to, from_label, e_label, to_label }
    }
}
