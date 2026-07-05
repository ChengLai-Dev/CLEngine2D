from table_exporter import TableDef

config = TableDef(
    source="assets/tables/道具表.xlsx",
    sheet="装备表",
    key="id",
    name="equipment_data",
    fields=[
        ("id", "id", int),
        ("名字", "name", str),
        ("价格", "price", int),
        ("防御", "defense", int),
    ],
)


@config.validate
def _(item):
    assert item["price"] >= 0, "价格不能为负"
    assert item["defense"] >= 0, "防御不能为负"
