#ifndef MO2_PYTHON_LOGGER_H
#define MO2_PYTHON_LOGGER_H

#include <boost/python.hpp>

/**
 * @brief Configure logging for MO2 python plugin.
 *
 * @return the log handler class.
 */
boost::python::object configure_python_logging();

#endif