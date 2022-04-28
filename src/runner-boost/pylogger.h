#ifndef MO2_PYTHON_LOGGER_H
#define MO2_PYTHON_LOGGER_H

#include <boost/python.hpp>

/**
 * @brief Configure logging for MO2 python plugin.
 *
 * @param mobase The mobase module.
 */
void configure_python_logging(boost::python::object mobase);

#endif