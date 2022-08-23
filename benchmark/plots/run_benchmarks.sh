repetitions=10

../../cmake-build-release/benchmark/comparison/benchmark-s3rvac-cpp-bencoding \
   --benchmark_repetitions=$repetitions \
   --benchmark_out_format=json \
   --benchmark_out="benchmark-s3rvac-cpp-bencoding.json"

../../cmake-build-release/benchmark/comparison/benchmark-jimporter-bencode \
   --benchmark_repetitions=$repetitions \
   --benchmark_out_format=json \
   --benchmark_out="benchmark-jimporter-bencode.json"

../../cmake-build-release/benchmark/comparison/benchmark-jimporter-bencode-boost \
   --benchmark_repetitions=$repetitions \
   --benchmark_out_format=json \
   --benchmark_out="benchmark-jimporter-bencode-boost.json"


../../cmake-build-release/benchmark/comparison/benchmark-arvidn-libtorrent \
   --benchmark_repetitions=$repetitions \
   --benchmark_out_format=json \
   --benchmark_out="benchmark-arvidn-libtorrent.json"

../../cmake-build-release/benchmark/comparison/benchmark-fbdtemme-bencode \
    --benchmark_repetitions=$repetitions \
    --benchmark_out_format=json \
    --benchmark_out="benchmark-fbdtemme-bencode.json"

../../cmake-build-release/benchmark/comparison/benchmark-rakshasa-libtorrent \
    --benchmark_repetitions=$repetitions \
    --benchmark_out_format=json \
    --benchmark_out="benchmark-rakshasa-libtorrent.json"

    ../../cmake-build-release/benchmark/comparison/benchmark-kriben-bencode \
    --benchmark_repetitions=$repetitions \
    --benchmark_out_format=json \
    --benchmark_out="benchmark-kriben-bencode.json"

../../cmake-build-release/benchmark/comparison/benchmark-theanti9-cppbencode \
    --benchmark_repetitions=$repetitions \
    --benchmark_out_format=json \
    --benchmark_out="benchmark-theanti9-cppbencode.json"


../../cmake-build-release/benchmark/comparison/benchmark-outputenable-bencode \
    --benchmark_repetitions=$repetitions \
    --benchmark_out_format=json \
    --benchmark_out="benchmark-outputenable-bencode.json"

../../cmake-build-release/benchmark/comparison/benchmark-aetf-qbencode \
    --benchmark_repetitions=$repetitions \
    --benchmark_out_format=json \
    --benchmark_out="benchmark-aetf-qbencode.json"

../../cmake-build-release/benchmark/comparison/benchmark-irajul-bencode \
    --benchmark_repetitions=$repetitions \
    --benchmark_out_format=json \
    --benchmark_out="benchmark-irajul-bencode.json"

../../cmake-build-release/benchmark/comparison/benchmark-s3ponia-bencodeparser \
    --benchmark_repetitions=$repetitions \
    --benchmark_out_format=json \
    --benchmark_out="benchmark-s3ponia-bencodeparser.json"