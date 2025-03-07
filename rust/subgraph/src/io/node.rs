/*
 * Copyright (c), Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug, Clone)]
#[allow(non_snake_case)]
pub struct Node {
    pub name: String,
    // #[serde(rename = "opType")]
    // pub op_type: String,
    pub opType: String,
    pub input: Vec<String>,
}
