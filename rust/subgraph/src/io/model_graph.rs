/*
 * Copyright (c), Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
use std::collections::HashMap;

use serde::{Deserialize, Serialize};

use crate::io::node::Node;

#[derive(Serialize, Deserialize, Clone)]
pub struct ModelGraph {
    pub name: String,
    pub nodes: HashMap<String, Node>,
    pub edges: Vec<(String, String)>,
    pub parameters: HashMap<String, String>,
}
