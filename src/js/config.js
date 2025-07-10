module.exports = [
    {
        "type": "heading",
        "defaultValue": "Simple Digital Configuration"
    },
    {
        "type": "text",
        "defaultValue": "The cleanest digital watchface around."
    },
    {
        "type": "section",
        "items": [
        {
            "type": "heading",
            "defaultValue": "Colors"
        },
        {
            "label": "Background Color",
            "type": "color",
            "messageKey": "BackgroundColor",
            "defaultValue": "0x000000",
            "allowGray": true
        },
        {
            
            "label": "Text Color",
            "type": "color",
            "messageKey": "ForegroundColor",
            "defaultValue": "0xFFFFFF",
            "allowGray": true
        }
        ]
    },
	{
		"type": "submit",
		"defaultValue": "Save"
	}
];