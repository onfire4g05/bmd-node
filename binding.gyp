{
	"targets": [
		{
			"target_name": "decklink",
			"sources": [
				"src/decklink.cpp",
			],
			"include_dirs": [
				"<!(node -p \"require('node-addon-api').include\")",
				"<!(node -p \"require('node-addon-api').include_dir\")",
			],
			"libraries": [],
			"cflags_cc": [ "-std=c++17" ],
			"defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
			"conditions": [
				["OS=='mac'", {
					"sources": [
						"lib/bmd-decklink-sdk/mac/DeckLinkAPIDispatch.cpp"
					],
					"include_dirs": [
						"lib/bmd-decklink-sdk/mac"
					],
					"libraries": [
						"-framework CoreFoundation"
					]
				}],
				["OS=='linux'", {
					"sources": [
						"lib/bmd-decklink-sdk/linux/DeckLinkAPIDispatch.cpp"
					],
					"include_dirs": [
						"lib/bmd-decklink-sdk/linux"
					],
					"libraries": [
						"-lDeckLinkAPI"
					]
				}],
			]
		}
	]
}
