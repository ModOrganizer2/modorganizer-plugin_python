#include "sipapiaccess.h"
#include <boost/python.hpp>
#include <QString>
#include <utility.h>

const sipAPIDef* sipAPIAccess::sipAPI()
{
    return (const sipAPIDef*)PyCapsule_Import("sip._C_API", 0);
}