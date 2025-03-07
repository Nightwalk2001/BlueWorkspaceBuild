/*
 * Copyright (c), Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
use std::sync::mpsc::Receiver;

use super::Config;
use crate::gspan::result::JSONResult;

pub trait MiningStrategy {
    fn run(&self, config: Config) -> Vec<JSONResult>;

    fn run_channel(&self, config: Config) -> Receiver<String>;
}
