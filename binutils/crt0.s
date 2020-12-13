.section start_section
__entry:
.export __entry
litw s 0xffff
call $main
halt
.import main
