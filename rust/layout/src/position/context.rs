use std::{
    ops,
    ops::{Index, IndexMut},
};

use ahash::HashMap;

use self::{Direction::*, Horizontal::*, Vertical::*};
use crate::Key;

#[derive(Copy, Clone, PartialEq)]
pub(super) enum Horizontal {
    Left,
    Right,
}

#[derive(Copy, Clone, PartialEq)]
pub(super) enum Vertical {
    Top,
    Bottom,
}

#[derive(Copy, Clone, Hash, Eq, PartialEq)]
pub(super) enum Direction {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
}

impl From<usize> for Direction {
    fn from(value: usize) -> Self {
        match value {
            0 => TopLeft,
            1 => TopRight,
            2 => BottomLeft,
            3 => BottomRight,
            _ => unreachable!(),
        }
    }
}

impl ops::Add<Horizontal> for Vertical {
    type Output = Direction;

    fn add(self, rhs: Horizontal) -> Self::Output {
        match (self, rhs) {
            (Top, Left) => TopLeft,
            (Top, Right) => TopRight,
            (Bottom, Left) => BottomLeft,
            (Bottom, Right) => BottomRight,
        }
    }
}

impl Direction {
    pub(super) fn horizon(self) -> Horizontal {
        match self {
            TopLeft | TopRight => Left,
            BottomLeft | BottomRight => Right,
        }
    }
}

type DirectionMap = [HashMap<Key, f32>; 4];

impl Index<Direction> for DirectionMap {
    type Output = HashMap<Key, f32>;

    fn index(&self, index: Direction) -> &Self::Output {
        match index {
            TopLeft => &self[0],
            TopRight => &self[1],
            BottomLeft => &self[2],
            BottomRight => &self[3],
        }
    }
}

impl IndexMut<Direction> for DirectionMap {
    fn index_mut(&mut self, index: Direction) -> &mut Self::Output {
        match index {
            TopLeft => &mut self[0],
            TopRight => &mut self[1],
            BottomLeft => &mut self[2],
            BottomRight => &mut self[3],
        }
    }
}

#[derive(Default)]
pub(super) struct Context {
    pub conflicts: HashMap<Key, Vec<Key>>,
    pub direction_map: DirectionMap,
    pub root: HashMap<Key, Key>,
    pub align: HashMap<Key, Key>,
    pub balanced: HashMap<Key, f32>
}
