use ahash::HashMap;

use crate::Key;

#[derive(Copy, Clone)]
pub(super) struct Barycenter {
    pub key: Key,
    pub barycenter: Option<f32>,
    pub weight: Option<f32>,
}

#[derive(Copy, Clone)]
pub struct ResolvedEntry {
    /// ## Layout
    /// This struct has 3 bytes of padding. If set either [`idx`]
    /// or [`indegree`] to u8, the size of the struct be reduced
    /// by 4 bytes, but *2^8-1* may not be enough.
    ///
    /// ## Another Solution
    /// Stores these three fields into u32. [`merged`] occupies
    /// 1 byte, [`idx`] and [`degree`] each occupies 15 bytes.
    pub idx: u16,
    pub indegree: u16,
    pub merged: bool,
    pub barycenter: Option<f32>,
    pub weight: Option<f32>,
}

impl ResolvedEntry {
    #[inline]
    pub(super) fn of(idx: u16) -> Self {
        Self { idx, indegree: 0, merged: false, barycenter: None, weight: None }
    }
}

/// A context for managing [`ResolvedEntry`] sources, sinks, and keys.
///
/// Storing these fields directly within `ResolvedEntry` would prevent
/// it from deriving the [`Copy`] trait. Additionally, it'd lead to
/// excessive cloning when accessing these fields.
///
/// ## Performance
/// There are three more solutions, Raw Pointer preferred.
/// - **Reference** Safe Rust, Manual lifecycle management, High Performance
/// - **RC/RefCell** Safe Rust, with extremely cumbersome boilerplate code.
/// - **NonNull** The best Performance, but Unsafe Rust,
#[derive(Default)]
pub(super) struct Context {
    pub entries: HashMap<Key, ResolvedEntry>,
    pub sources_map: HashMap<Key, Vec<Key>>,
    pub sinks_map: HashMap<Key, Vec<Key>>,
    pub keys_map: HashMap<Key, Vec<Key>>,
    pub keys: Vec<Key>,
    pub index: usize,
}
