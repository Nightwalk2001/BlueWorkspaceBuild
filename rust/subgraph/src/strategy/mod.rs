pub mod mining_strategy;
pub mod gspan_mining;
pub mod config;

use std::sync::mpsc::Receiver;
use config::Config;

use crate::gspan::result::JSONResult;

use self::mining_strategy::MiningStrategy;

pub struct MiningContext {
    strategy: Box<dyn MiningStrategy>,
}

impl MiningContext {
    pub fn new(strategy: Box<dyn MiningStrategy>) -> Self {
        MiningContext { strategy }
    }

    #[allow(dead_code)]
    pub fn run(&self, config: Config) -> Vec<JSONResult> {
        self.strategy.run(config)
    }

    #[allow(dead_code)]
    pub fn run_channel(&self, config: Config) -> Receiver<String> {
        self.strategy.run_channel(config)
    }
}