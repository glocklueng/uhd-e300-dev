# The package naming convention is <core_name>_xmdf
package provide ddr_ila_wrpath_xmdf 1.0

# This includes some utilities that support common XMDF operations
package require utilities_xmdf

# Define a namespace for this package. The name of the name space
# is <core_name>_xmdf
namespace eval ::ddr_ila_wrpath_xmdf {
# Use this to define any statics
}

# Function called by client to rebuild the params and port arrays
# Optional when the use context does not require the param or ports
# arrays to be available.
proc ::ddr_ila_wrpath_xmdf::xmdfInit { instance } {
# Variable containing name of library into which module is compiled
# Recommendation: <module_name>
# Required
utilities_xmdf::xmdfSetData $instance Module Attributes Name ddr_ila_wrpath
}
# ::ddr_ila_wrpath_xmdf::xmdfInit

# Function called by client to fill in all the xmdf* data variables
# based on the current settings of the parameters
proc ::ddr_ila_wrpath_xmdf::xmdfApplyParams { instance } {

set fcount 0
# Array containing libraries that are assumed to exist
# Examples include unisim and xilinxcorelib
# Optional
# In this example, we assume that the unisim library will
# be available to the simulation and synthesis tool
utilities_xmdf::xmdfSetData $instance FileSet $fcount type logical_library
utilities_xmdf::xmdfSetData $instance FileSet $fcount logical_library unisim
incr fcount


utilities_xmdf::xmdfSetData $instance FileSet $fcount relative_path ddr_ila_wrpath.asy
utilities_xmdf::xmdfSetData $instance FileSet $fcount type asy
incr fcount

utilities_xmdf::xmdfSetData $instance FileSet $fcount relative_path ddr_ila_wrpath.cdc
utilities_xmdf::xmdfSetData $instance FileSet $fcount type AnyView
incr fcount


utilities_xmdf::xmdfSetData $instance FileSet $fcount relative_path ddr_ila_wrpath.constraints/ddr_ila_wrpath.ucf
utilities_xmdf::xmdfSetData $instance FileSet $fcount type Ignore
incr fcount


utilities_xmdf::xmdfSetData $instance FileSet $fcount relative_path ddr_ila_wrpath.constraints/ddr_ila_wrpath.xdc
utilities_xmdf::xmdfSetData $instance FileSet $fcount type Xdc
incr fcount
utilities_xmdf::xmdfSetData $instance FileSet $fcount relative_path ddr_ila_wrpath.ncf
utilities_xmdf::xmdfSetData $instance FileSet $fcount type Ncf
incr fcount


utilities_xmdf::xmdfSetData $instance FileSet $fcount relative_path ddr_ila_wrpath.ngc
utilities_xmdf::xmdfSetData $instance FileSet $fcount type ngc
incr fcount

utilities_xmdf::xmdfSetData $instance FileSet $fcount relative_path ddr_ila_wrpath.vhd
utilities_xmdf::xmdfSetData $instance FileSet $fcount type vhdl
incr fcount

utilities_xmdf::xmdfSetData $instance FileSet $fcount relative_path ddr_ila_wrpath.vho
utilities_xmdf::xmdfSetData $instance FileSet $fcount type vhdl_template
incr fcount

utilities_xmdf::xmdfSetData $instance FileSet $fcount relative_path ddr_ila_wrpath.xco
utilities_xmdf::xmdfSetData $instance FileSet $fcount type coregen_ip
incr fcount

utilities_xmdf::xmdfSetData $instance FileSet $fcount relative_path ddr_ila_wrpath_xmdf.tcl
utilities_xmdf::xmdfSetData $instance FileSet $fcount type AnyView
incr fcount

utilities_xmdf::xmdfSetData $instance FileSet $fcount associated_module ddr_ila_wrpath
incr fcount

}

# ::gen_comp_name_xmdf::xmdfApplyParams

