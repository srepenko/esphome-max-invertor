import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import ICON_POWER
from .. import INVERTER_COMPONENT_SCHEMA, inverter_ns


DEPENDENCIES = ["uart"]

CONF_OUTPUT_SOURCE_PRIORITY_UTILITY = "output_source_priority_utility"
CONF_OUTPUT_SOURCE_PRIORITY_SOLAR = "output_source_priority_solar"
CONF_OUTPUT_SOURCE_PRIORITY_BATTERY = "output_source_priority_battery"
CONF_INPUT_VOLTAGE_RANGE = "input_voltage_range"
CONF_PV_OK_CONDITION_FOR_PARALLEL = "pv_ok_condition_for_parallel"
CONF_PV_POWER_BALANCE = "pv_power_balance"

TYPES = {
    CONF_OUTPUT_SOURCE_PRIORITY_UTILITY: ("POP00", None),
    CONF_OUTPUT_SOURCE_PRIORITY_SOLAR: ("POP01", None),
    CONF_OUTPUT_SOURCE_PRIORITY_BATTERY: ("POP02", None),
    CONF_INPUT_VOLTAGE_RANGE: ("PGR01", "PGR00"),
    CONF_PV_OK_CONDITION_FOR_PARALLEL: ("PPVOKC1", "PPVOKC0"),
    CONF_PV_POWER_BALANCE: ("PSPB1", "PSPB0"),
}

InverterSwitch = inverter_ns.class_("InverterSwitch", switch.Switch, cg.Component)

SWITCH_SCHEMA = switch.switch_schema(
    InverterSwitch, icon=ICON_POWER, block_inverted=True
).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = INVERTER_COMPONENT_SCHEMA.extend(
    {cv.Optional(type): SWITCH_SCHEMA for type in TYPES}
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_INVERTER_ID])

    for type, (on, off) in TYPES.items():
        if type in config:
            conf = config[type]
            var = await switch.new_switch(conf)
            await cg.register_component(var, conf)
            cg.add(getattr(paren, f"set_{type}_switch")(var))
            cg.add(var.set_parent(paren))
            cg.add(var.set_on_command(on))
            if off is not None:
                cg.add(var.set_off_command(off))
