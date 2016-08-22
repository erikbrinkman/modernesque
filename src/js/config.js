module.exports = [
  {
    "type": "heading",
    "defaultValue": "Modernesque Configuration"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Display"
      },
      {
        "type": "toggle",
        "messageKey": "SHOW_DATE",
        "defaultValue": true,
        "label": "Show Date"
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Colors"
      },
      {
        "type": "color",
        "messageKey": "MINUTE_COLOR",
        "defaultValue": 0x000000,
        "label": "Minute Color"
      },
      {
        "type": "color",
        "messageKey": "MINUTE_BACKGROUND_COLOR",
        "defaultValue": 0xaaaaaa,
        "label": "Minute Background Color"
      },
      {
        "type": "color",
        "messageKey": "HOUR_COLOR",
        "defaultValue": 0x000000,
        "label": "Hour Color"
      },
      {
        "type": "color",
        "messageKey": "DATE_COLOR",
        "defaultValue": 0x000000,
        "label": "Date Color"
      },
      {
        "type": "color",
        "messageKey": "BACKGROUND_COLOR",
        "defaultValue": 0xffffff,
        "label": "Background Color"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];