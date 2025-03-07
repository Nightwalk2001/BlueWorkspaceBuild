/*
 * Copyright (c), Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
pub mod config;
pub mod gspan_mining;
pub mod mining_strategy;

use std::sync::mpsc::Receiver;

use config::Config;

use self::mining_strategy::MiningStrategy;
use crate::gspan::result::JSONResult;

pub struct MiningContext {
    strategy: Box<dyn MiningStrategy>,
}

impl MiningContext {
    pub fn new(strategy: Box<dyn MiningStrategy>) -> Self {
        MiningContext { strategy }
    }

    pub fn run(&self, config: Config) -> Vec<JSONResult> {
        self.strategy.run(config)
    }

    pub fn run_channel(&self, config: Config) -> Receiver<String> {
        self.strategy.run_channel(config)
    }
}
