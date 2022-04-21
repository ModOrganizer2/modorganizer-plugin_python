#ifndef MO2_PYTHON_LOGGER_H
#define MO2_PYTHON_LOGGER_H

#include <pybind11/pybind11.h>

/**
 * @brief Configure logging for MO2 python plugin.
 *
 * @param mobase The mobase module.
 */
void configure_python_logging(pybind11::module_ mobase);

#endif
