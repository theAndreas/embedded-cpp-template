// The paragraph of a command is rewrapped to 80 columns: a line too long is
// broken, short lines are joined.

/*!
 * \pre `buffer` holds at least `size` bytes, and nothing else writes to it
 *      while this runs, not even an interrupt.
 * \note Short lines are joined.
 * \warning Once a word is longer than what fits:
 *          Abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz
 */
void Write(char* buffer, int size);
