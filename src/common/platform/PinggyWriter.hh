#ifndef __SRC_CPP_PUBLIC_COMMON_PLATFORM_PINGGY_WRITER_HH__
#define __SRC_CPP_PUBLIC_COMMON_PLATFORM_PINGGY_WRITER_HH__

#include "SharedPtr.hh"
#include <utils/RawData.hh>

namespace common
{

abstract class PingyWriter: virtual public pinggy::SharedObject
{
public:
    virtual
    ~PingyWriter()
                                { }

    virtual ssize_t
    Write(RawDataPtr rwData, int flags = 0) = 0;

    virtual ssize_t
    Write(const void *buf, size_t nbyte, int flags = 0) = 0;

    DefineMandatoryAbsClassFunctionsWOSuperNoDump(PingyWriter);
};
DeclareSharedPtr(PingyWriter);

} // namespace common



#endif // __SRC_CPP_PUBLIC_COMMON_PLATFORM_INTERFACES_HH__