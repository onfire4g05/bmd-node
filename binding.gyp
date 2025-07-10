{
  "targets": [
    {
      "target_name": "decklink_temp",
      "sources": [
		  "decklink_temp.cpp",
		  "lib/bmd-decklink-sdk/DeckLinkAPIDispatch.cpp"
 	],
      "include_dirs": [
        "<!(node -p \"require('node-addon-api').include\")",
        "<!(node -p \"require('node-addon-api').include_dir\")",
        "lib/bmd-decklink-sdk"
      ],
      "libraries": [ "-lDeckLinkAPI" ],
      "cflags_cc": [ "-std=c++17" ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ]
    }
  ]
}
