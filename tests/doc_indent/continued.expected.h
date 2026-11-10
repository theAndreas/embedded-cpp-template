// A line that continues a command, started at the margin by clang-format, is
// indented under the text after the command.

/*!
 * \brief Returns the element at `index`, or IndexError::kOutOfRange if it lies
 *        beyond the end.
 *
 * \tparam Element The type of what the view holds, which the caller chooses.
 * \param[in] index The position, from 0, which has to lie below size().
 * \param[in,out] buffer What is written, and where.
 * \retval kOutOfRange `index` lies beyond the end of the view.
 * \return The view of them, or IndexError::kOutOfRange if `position` lies
 *         beyond the end.
 */
int At(int index);
