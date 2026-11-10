// What is not the paragraph of a command in a /*! block is left as it is, even
// where it is indented otherwise.

/*! \brief A block of one line, with a \return in it. */
int One();

/*! \brief A block whose end stands after its text.
 *  Its block ends there, and what follows is not in it. */
int Two();

/**
 * \return A block that starts with /**, not /*!, is not Doxygen of ours
 * and stays.
 */
int Three();

// \return A comment of lines
// stays.
int Four();

/*!
 * \brief A block with an example.
 *
 * \code
 * \return Inside the example, a command is code
 * and stays.
 * \endcode
 *
 * \returns Not a command of the list, nor \precondition, nor @return
 * which is never used.
 */
int Five();
