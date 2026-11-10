// An example, \code ... \endcode, is left as it is, also where the line that
// opens the block opens it; one without its \endcode ends with its block, and
// the next block is indented again.

/*! \code
 * \return Inside an example that the first line opens, a command
 * is code and stays.
 * \endcode
 */
int One();

/*!
 * \brief An example that is never closed.
 * \code
 * \return Code
 * as well.
 */
int Two();

/*!
 * \return The block after it, whose command is indented.
 */
int Three();
