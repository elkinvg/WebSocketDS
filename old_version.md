## server mode attributes

```json
{
	"event": "read",
	"type_req":"attribute",
	"data": [
		{
			"attr": "attribute name",
			"data": "data",
			"set": "if writable attribute"
		}
	],
	"pipe":
		{
			"attrName1" : "Data in format dependent on type",
			"attrName2" : ["Data in format", "dependent on type"]
		}
}
```

## server mode attributes for group

```json
{
	"event": "read",
	"type_req":"group_attribute",
	"data":
	{
		"name/tango/device_from_group" :
		[
			{
				"attr": "attribute name",
				"data": "data",
				"set": "if writable attribute"
			}
		]
	},
	"pipe":
		{
			"nameof/tango/device_from_group":
			{
				"attrName1" : "Data in format dependent on type",
				"attrName2" : ["Data in format", "dependent on type"]
			}
		}
}
```

## data from timer

```json
{
	"event": "read",
	"type_req": "from_timer",
	"data": {
		"name/of/tangodevice": {
			"attrs": [{
				"attr": "имя атрибута",
				"data": "значение"
			}],
			"pipe": {
				"attribute_name": "value",
				"other_attribute": ["value", "value"]
			}
		}
	}
}
```

## client mode attributes

For device:

```json
{
	"event": "read",
	"type_req": "attr_device_cl",
	"device_name": "name of device or alias",
	"id_req": "request id",
	"data": {
		"attrs":
			[
				{
				"attr": "attribute name",
				"data": "data",
				"set": "if writable attribute"
				}
			],
		"pipe": {
				"attribute_name": "value",
				"other_attribute": ["value", "value"]
		}
	}
}
```

For group:

```json
{
	"event": "read",
	"type_req": "attr_group_cl",
	"id_req": "Request id",
	"data": {
		"attrs": {
			"device_name": [
				{
				"attr": "attribute name",
				"data": "data",
				"set": "if writable attribute"
				}
			]
		},			
		"pipe": {
			"device_name": {
				"attribute_name": "value",
				"other_attribute": ["value", "value"]
			}
		}
	}
}
```