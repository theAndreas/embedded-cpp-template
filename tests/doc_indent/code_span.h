// A code span in backticks is one word: it moves to the next line whole. A
// backtick that opens none is a character of its word and stays.

/*!
 * \return The value of the register, as read with `sbi(PORTB, PB5)` before.
 * \see `operator[](size_type index) const`, which does not check the index at all.
 * \note A lone ` stays where it stands.
 */
int Read();
