from table_exporter import TableDef

config = TableDef(
    source="assets/tables/道具表.xlsx",
    sheet="武器表",
    key="id",
    name="weapon_data",
    fields=[
        ("id", "id", int),
        ("名字", "name", str),
        ("价格", "price", int),
        ("伤害", "damage", float),
        ("可装备", "is_weapon", bool),
    ],
)


@config.validate
def _(item):
    assert item["price"] >= 0, "价格不能为负"
    assert item["damage"] >= 0, "伤害不能为负"
 