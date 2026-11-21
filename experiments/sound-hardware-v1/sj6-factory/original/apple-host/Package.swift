// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "SuperJ6Reference",
    platforms: [.iOS(.v15), .macOS(.v12)],
    products: [.library(name: "ReferenceBench", targets: ["ReferenceBench"])],
    targets: [
        .target(name: "ReferenceBench"),
        .testTarget(name: "ReferenceBenchTests", dependencies: ["ReferenceBench"],
                    resources: [.copy("Inputs")]),
    ]
)
