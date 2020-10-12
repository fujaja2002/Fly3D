set(Runtime_TLSF_HDRS
	Runtime/TLSF/tlsf.h
)
set(Runtime_TLSF_SRCS
	Runtime/TLSF/tlsf.c
)

add_library(FlyCore STATIC
	${Runtime_TLSF_HDRS}
	${Runtime_TLSF_SRCS}
)

source_group(Runtime\\TLSF FILES ${Runtime_TLSF_HDRS} ${Runtime_TLSF_SRCS})