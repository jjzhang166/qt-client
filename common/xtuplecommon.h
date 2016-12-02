#include <QDebug>
#define SIGTRACK false
#define FUNTRACK false

/**	@addtogroup globals Global Variables and Functions
	@{
	*/
/**
	@def SIGTRACK
	Add "#define SIGTRACK true" to any file that inherits @c guiclient.h or @c widgets.h to enable Signature tracking by the @c TATTLE macro.
	Please note, this must be added below the @c include statements for @c guiclient.h or @c widgets.h , or the value will be reset to @c false.
*/
/**
	@def FUNTRACK
	Add "#define FUNTRACK true" to any file that inherits @c guiclient.h or @c widgets.h to enable Function tracking by the @c TATTLE macro.
	Please note, this must be added below the @c include statements for @c guiclient.h or @c widgets.h , or the value will be reset to @c false.
*/
/**
	@def TATTLE
	Use in classes that inherit @c guiclient.h or @c widgets.h .
	TATTLE may be written after the opening bracket of most member functions to allow signal and function call information to be written to the output stream.
	If only @c SIGTRACK is defined true for a class, then only member functions that are called by a Qt Signal will be written to the output stream with information about the sender.
	If only @c FUNTRACK is defined true for a class, then all member functions that are called will be written to the output stream.
	If both @c SIGTRACK and @c FUNTRACK are defined true for a class, then all member functions that are called will be written to the output stream, but functions that
	are called by a Qt Signal will include additional information about the sender of the Signal.
*/
/** @} */

#define TATTLE                                          \
{                                                       \
    if (SIGTRACK && sender())                           \
    {                                                   \
        qDebug() << "EZGREP..."                         \
                 << "QObject:"  << sender()             \
                 << "Index:"    << senderSignalIndex()  \
                 << "calls "    << __FILE__ << "::"     \
                                << __func__ << "()";    \
    }                                                   \
    else if (SIGTRACK && FUNTRACK)                      \
    {                                                   \
        qDebug() << "EZGREP..."                         \
		 << QString().sprintf("%-8p", this)	\
                 << __FILE__ << "::"                    \
                 << __func__ << "()";                   \
    }                                                   \
    if (!SIGTRACK && FUNTRACK)                          \
    {                                                   \
        qDebug() << "EZGREP..."                         \
		 << QString().sprintf("%-8p", this)	\
                 << __FILE__ << "::"                    \
                 << __func__ << "()";                   \
    }                                                   \
}
