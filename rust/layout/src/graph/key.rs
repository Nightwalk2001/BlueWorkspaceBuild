/// ## Performance
/// #### String
/// it's challenging to avoid a large number of clone operations,
/// and performance deteriorates sharply.
/// #### SmartString
/// [`SmartString::clone`] usually copies 24 bytes with [`clone`] everywhere.
/// In addition, [`SmartString`] doesn't derive [`Copy`] trait.
/// #### usize
/// only 8 bytes need to be copied.
/// All parameters are passed by value,
/// and the number of clones in the code is greatly reduced.
/// At the same time, most struct can also derive the [`Copy`] trait.
pub type Key = usize;

#[inline]
pub fn normalize_st(s: Key, t: Key) -> (Key, Key) {
    if t < s { (t, s) } else { (s, t) }
}

pub trait KeyCodecExt {
    fn of(source: Key, target: Key) -> Key;

    fn source(self) -> Key;

    fn target(self) -> Key;

    fn decode(self) -> (Key, Key);
}

/// # Memory Layout
///
/// The **Key** is represented using 64 bits, with the following layout:
///
/// | Field | Size (bits) | Description |
/// |-----------------|-------------|--------------------------------------------------|
/// | Reserved | 16 | Reserved and unused.|
/// | Usable          | 48          | Used to store keys.|
/// |                 |             | - For `NodeKey`, it occupies the higher 24 bits. |
/// |                 |             | - For `EdgeKey`, source and target each occupy 24 bits.|
///
/// This allows handling up to 16 million nodes, which is enough for all scenarios.
impl KeyCodecExt for Key {
    #[inline]
    fn of(source: Key, target: Key) -> Key {
        source.wrapping_shl(24) + target
    }

    #[inline]
    fn source(self) -> Key {
        self >> 24
    }

    #[inline]
    fn target(self) -> Key {
        self & 0xFFFFFF
    }

    #[inline]
    fn decode(self) -> (Key, Key) {
        let s = self >> 24;
        let t = self & 0xFFFFFF;

        (s, t)
    }
}

pub const EMPTY_KEY: Key = 1 << 48;
pub const EMPTY_ROOT: Key = EMPTY_KEY + 1;
