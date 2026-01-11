cc_library(
    name = "server",
    srcs = ["http_server.cc"],
    hdrs = ["http_server.h"],
    deps = [
        "//epiphany/database:database",
        "//epiphany/qrs:qrs",
        "//epiphany/observability:metrics",
    ],
    visibility = ["//visibility:public"],
)
