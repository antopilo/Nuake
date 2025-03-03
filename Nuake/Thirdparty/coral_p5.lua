group "Thirdparty"
include "Coral/Coral.Native"
if not _OPTIONS["disable-dotnet"] then
    include "Coral/Coral.Managed"
end
group ""
