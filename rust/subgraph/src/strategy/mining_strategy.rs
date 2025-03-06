use std::sync::mpsc::Receiver;
use crate::gspan::result::JSONResult;

use super::Config;

pub trait MiningStrategy {
    #[allow(dead_code)]
    fn run(&self, config: Config) -> Vec<JSONResult>;

    #[allow(dead_code)]
    fn run_channel(&self, config: Config) -> Receiver<String>;
}
