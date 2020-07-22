// swift-tools-version:5.2
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "faxpp",
    products: [
        .library(
            name: "faxpp",
            targets: ["faxpp"]),
    ],
    targets: [
        .target(
            name: "faxpp",
            dependencies: [],
            path: ".",
            exclude: [
                "autotools",
                "docs",
                "examples",
                "Package.swift",
                "tests",
            ]),
    ]
)
