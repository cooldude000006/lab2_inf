"""Графики по результатам lab2_bench (DESIGN.md, п. 4.9).

Запуск:  python bench/plot.py [bench/results/results.csv]
Картинки сохраняются рядом с CSV-файлом:
  build_N.png  — время построения индекса от размера коллекции,
                 отдельно для данных по возрастанию, по убыванию и случайных;
  search_N.png — время поиска по ключу и по диапазону (среднее по порядкам данных);
где N — номер атрибута в порядке появления в CSV.
"""

import csv
import sys
from collections import defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")  # без окна: только запись в файлы
import matplotlib.pyplot as plt  # noqa: E402

METHOD_STYLES = {
    "linear": dict(color="#7f7f7f", marker="s", label="линейный поиск"),
    "binary": dict(color="#1f77b4", marker="o", label="бинарный поиск"),
    "avl": dict(color="#d62728", marker="^", label="АВЛ-дерево"),
}

ORDER_TITLES = {
    "ascending": "данные по возрастанию",
    "descending": "данные по убыванию",
    "random": "случайные данные",
}


def read_rows(path):
    with open(path, encoding="utf-8", newline="") as file:
        return list(csv.DictReader(file))


def series(rows, method, value_field):
    """Точки (размер, значение) метода; пропущенные замеры не попадают.

    Если для одного размера есть несколько замеров (разные порядки данных),
    берётся среднее.
    """
    values_by_size = defaultdict(list)
    for row in rows:
        if row["method"] == method and row["skipped"] == "0":
            values_by_size[int(row["size"])].append(float(row[value_field]))
    sizes = sorted(values_by_size)
    return sizes, [sum(values_by_size[s]) / len(values_by_size[s]) for s in sizes]


def draw(axis, rows, value_field, title, ylabel, skip_methods=()):
    for method, style in METHOD_STYLES.items():
        if method in skip_methods:
            continue
        sizes, values = series(rows, method, value_field)
        # На логарифмической шкале ноль не изобразить.
        pairs = [(s, v) for s, v in zip(sizes, values) if v > 0]
        if pairs:
            axis.plot([p[0] for p in pairs], [p[1] for p in pairs], **style)
    axis.set_xscale("log")
    axis.set_yscale("log")
    axis.set_title(title)
    axis.set_xlabel("число записей")
    axis.set_ylabel(ylabel)
    axis.grid(True, which="both", alpha=0.3)
    axis.legend()


def plot_attribute(number, attribute, rows, out_dir):
    figure, axes = plt.subplots(1, 3, figsize=(16, 5))
    for axis, (order, title) in zip(axes, ORDER_TITLES.items()):
        order_rows = [row for row in rows if row["order"] == order]
        # Линейному поиску строить нечего — на графике построения его нет.
        draw(axis, order_rows, "build_ms", title, "построение, мс", skip_methods=("linear",))
    figure.suptitle(f"Построение индекса: {attribute}")
    figure.tight_layout()
    figure.savefig(out_dir / f"build_{number}.png", dpi=120)
    plt.close(figure)

    # Время запроса от порядка исходных данных не зависит, поэтому оно
    # усредняется по всем порядкам, где индекс удалось построить.
    figure, axes = plt.subplots(1, 2, figsize=(12, 5))
    draw(axes[0], rows, "find_us", "поиск по ключу", "время запроса, мкс")
    draw(axes[1], rows, "range_us", "поиск по диапазону", "время запроса, мкс")
    figure.suptitle(f"Поиск: {attribute}")
    figure.tight_layout()
    figure.savefig(out_dir / f"search_{number}.png", dpi=120)
    plt.close(figure)


def main():
    # Консоль Windows по умолчанию не в UTF-8: без этого русские названия
    # атрибутов в выводе превращаются в «кракозябры».
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")
    default = Path(__file__).resolve().parent / "results" / "results.csv"
    path = Path(sys.argv[1]) if len(sys.argv) > 1 else default
    rows = read_rows(path)
    if not rows:
        print(f"В файле {path} нет замеров")
        return 1

    by_attribute = defaultdict(list)
    for row in rows:
        by_attribute[row["attribute"]].append(row)

    for number, (attribute, attribute_rows) in enumerate(by_attribute.items(), start=1):
        plot_attribute(number, attribute, attribute_rows, path.parent)
        print(f"{number}: {attribute} -> build_{number}.png, search_{number}.png")
    return 0


if __name__ == "__main__":
    sys.exit(main())
