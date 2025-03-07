/*
 * Copyright (c), Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
use std::sync::atomic::{AtomicUsize, Ordering};

#[derive(Debug, Clone)]
pub struct Edge {
    pub id: usize,
    pub from: String,
    pub to: String,
    pub from_label: String,
    pub to_label: String,
    pub e_label: String,
}

impl Edge {
    pub const NIL_E_LABEL: &'static str = "<NIL>";

    pub fn new(
        from: String,
        to: String,
        from_label: String,
        to_label: String,
        e_label: Option<String>,
    ) -> Edge {
        static COUNTER: AtomicUsize = AtomicUsize::new(1);
        Edge {
            id: COUNTER.fetch_add(1, Ordering::Relaxed),
            from,
            to,
            from_label,
            to_label,
            e_label: match e_label {
                None => String::from(Self::NIL_E_LABEL),
                Some(value) => value,
            },
        }
    }

    pub fn to_str_repr(&self) -> String {
        vec![
            "e".to_string(),
            self.from.to_string(),
            self.to.to_string(),
            self.from_label.to_string(),
            self.to_label.to_string(),
            self.e_label.to_string(),
        ]
        .join(" ")
    }
}

impl PartialEq for Edge {
    fn eq(&self, other: &Self) -> bool {
        self.from_label == other.from_label
            && self.to_label == other.to_label
            && self.e_label == other.e_label
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_equal_edge() {
        let edge1 = Edge::new(
            String::from("node_1"),
            String::from("node_2"),
            String::from("a"),
            String::from("b"),
            Some(String::from("A")),
        );
        let edge2 = Edge::new(
            String::from("node_1"),
            String::from("node_2"),
            String::from("a"),
            String::from("b"),
            Some(String::from("A")),
        );
        let edge3 = Edge::new(
            String::from("node_2"),
            String::from("node_1"),
            String::from("a"),
            String::from("b"),
            Some(String::from("C")),
        );

        assert_eq!(edge1, edge2);
        assert_ne!(edge1, edge3);
        assert_ne!(edge2, edge3);
    }
}
