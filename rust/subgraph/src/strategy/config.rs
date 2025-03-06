use std::path::Path;
use std::fs;

use crate::gspan::models::graph::Graph;
use crate::gspan::result::OutType;

#[derive(Debug)]
#[allow(dead_code)]
pub enum ConfigError {
    InvalidInputFile(String),
    InvalidProcessFile(String),
    InvalidOutputFile(String),
    MinSupportTooSmall(usize),
    MinInnerSupportTooSmall(usize),
    MinVerticesGreaterThanMax(usize, usize),
    NonNormalizedPath(String),
    SymlinkNotAllowed(String),
    IllegalCharacters(String),
}

#[derive(Debug)]
pub enum InputSource {
    File(String),
    Graphs(Vec<Graph>),
}

#[derive(Debug, Clone)]
pub enum ProcessPath {
    File(String),
    None,
}

#[derive(Debug)]
pub enum OutputPath {
    File(String),
    None,
}

#[derive(Debug)]
pub struct Config {
    input_source: InputSource,
    process_path: ProcessPath,
    output_path: OutputPath,
    output_type: OutType,
    min_support: usize,         // 相同结构在不同图中出现的最小次数
    min_inner_support: usize,   // 相同结构在图内部中出现的最小次数
    min_vertices: usize,        // Minimum number of vertices
    max_vertices: usize,        // Maximum number of vertices
}

impl Config {
    pub fn get_input_source(&self) -> &InputSource {
        &self.input_source
    }

    pub fn get_process_path(&self) -> &ProcessPath {
        &self.process_path
    }

    pub fn get_output_path(&self) -> &OutputPath {
        &self.output_path
    }

    pub fn get_output_type(&self) -> &OutType {
        &self.output_type
    }

    pub fn get_min_support(&self) -> usize {
        self.min_support
    }

    pub fn get_min_inner_support(&self) -> usize {
        self.min_inner_support
    }

    pub fn get_min_vertices(&self) -> usize {
        self.min_vertices
    }

    pub fn get_max_vertices(&self) -> usize {
        self.max_vertices
    }
}

impl Config {
    pub fn new(
        input_file: &str,
        process_file: Option<&str>,
        output_file: Option<&str>,
        output_type: OutType,
        min_support: usize,
        min_inner_support: usize,
        min_vertices: usize,
        max_vertices: usize,
    ) -> Result<Config, ConfigError> {
        // 校验文件路径是否为空
        if input_file.is_empty() {
            return Err(ConfigError::InvalidInputFile("Input file path cannot be empty.".to_string()));
        }

        check_normalized_path(&input_file)?;

        // 校验 process_file 和 output_file 是否为空
        if let Some(ref process_file) = process_file {
            if process_file.is_empty() {
                return Err(ConfigError::InvalidProcessFile("Process file path cannot be empty.".to_string()));
            }
            check_normalized_path(process_file)?;
        }

        if let Some(ref output_file) = output_file {
            if output_file.is_empty() {
                return Err(ConfigError::InvalidOutputFile("Output file path cannot be empty.".to_string()));
            }
            check_normalized_path(output_file)?;
        }

        // 校验支持度阈值
        if min_support < 1 {
            return Err(ConfigError::MinSupportTooSmall(min_support));
        }
        if min_inner_support < 1 {
            return Err(ConfigError::MinInnerSupportTooSmall(min_inner_support));
        }

        // 校验顶点数阈值
        if min_vertices > max_vertices {
            return Err(ConfigError::MinVerticesGreaterThanMax(min_vertices, max_vertices));
        }

        Ok(Config {
            input_source: InputSource::File(input_file.to_string()),
            process_path: process_file.map_or(ProcessPath::None, |file| ProcessPath::File(file.to_string())),
            output_path: output_file.map_or(OutputPath::None, |file| OutputPath::File(file.to_string())),
            output_type,
            min_support,
            min_inner_support,
            min_vertices,
            max_vertices,
        })
    }

    pub fn new_from_graphs(
        graphs: Vec<Graph>,
        process_file: Option<&str>,
        output_file: Option<&str>,
        output_type: OutType,
        min_support: usize,
        min_inner_support: usize,
        min_vertices: usize,
        max_vertices: usize,
    ) -> Result<Config, ConfigError> {

        // 校验 process_file 和 output_file 是否为空
        if let Some(ref process_file) = process_file {
            if process_file.is_empty() {
                return Err(ConfigError::InvalidProcessFile("Process file path cannot be empty.".to_string()));
            }
            check_normalized_path(process_file)?;
        }

        if let Some(ref output_file) = output_file {
            if output_file.is_empty() {
                return Err(ConfigError::InvalidOutputFile("Output file path cannot be empty.".to_string()));
            }
            check_normalized_path(output_file)?;
        }

        // 校验支持度阈值
        if min_support < 1 {
            return Err(ConfigError::MinSupportTooSmall(min_support));
        }
        if min_inner_support < 1 {
            return Err(ConfigError::MinInnerSupportTooSmall(min_inner_support));
        }

        // 校验顶点数阈值
        if min_vertices > max_vertices {
            return Err(ConfigError::MinVerticesGreaterThanMax(min_vertices, max_vertices));
        }

        Ok(Config {
            input_source: InputSource::Graphs(graphs),
            process_path: process_file.map_or(ProcessPath::None, |file| ProcessPath::File(file.to_string())),
            output_path: output_file.map_or(OutputPath::None, |file| OutputPath::File(file.to_string())),
            output_type,
            min_support,
            min_inner_support,
            min_vertices,
            max_vertices,
        })
    }
}

// 校验路径是否已标准化（不允许是软链接，且没有非法字符）
fn check_normalized_path(path: &str) -> Result<(), ConfigError> {
    let p = Path::new(path);
    
    // 检查是否为符号链接
    if fs::symlink_metadata(p).and_then(|metadata| Ok(metadata.file_type().is_symlink())).unwrap_or(false) {
        return Err(ConfigError::SymlinkNotAllowed(format!("Path '{}' is a symbolic link.", path)));
    }

    // 检查是否存在非法字符
    if let Some(illegal_char) = find_illegal_characters(path) {
        return Err(ConfigError::IllegalCharacters(format!("Path '{}' contains illegal character '{}'.", path, illegal_char)));
    }

    Ok(())
}

fn find_illegal_characters(path: &str) -> Option<char> {
    // 定义非法字符集合
    let illegal_chars = ['<', '>', ':', '"', '|', '?', '*'];

    for c in path.chars() {
        if illegal_chars.contains(&c) {
            return Some(c);
        }
    }

    None
}

// 测试代码
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_valid_config() {
        let config_result = Config::new(
            "tests/data/input.txt",
            Some("tests/data/process.txt"),
            Some("tests/data/output.txt"),
            OutType::JSON,
            2,
            1,
            3,
            5,
        );
        assert!(config_result.is_ok());
    }

    #[test]
    fn test_illegal_characters_in_path() {
        let config_result = Config::new(
            "tests/data/illegal*name.txt",
            Some("tests/data/process.txt"),
            Some("tests/data/output.txt"),
            OutType::JSON,
            2,
            1,
            3,
            5,
        );
        assert!(matches!(config_result.unwrap_err(), ConfigError::IllegalCharacters(_)));
    }

    // #[test]
    // fn test_symlink_not_allowed() {
    //     // 假设 "tests/data/symlink_input.txt" 是一个指向 "tests/data/input.txt" 的符号链接
    //     let config_result = Config::new(
    //         "tests/data/symlink_input.txt".to_string(),
    //         "tests/data/process.txt".to_string(),
    //         "tests/data/output.txt".to_string(),
    //         OutType::Json,
    //         2,
    //         1,
    //         3,
    //         5,
    //     );
    //     assert!(matches!(config_result.unwrap_err(), ConfigError::SymlinkNotAllowed(_)));
    // }
}