#!@Python3_EXECUTABLE@
import re
import sys
import argparse
import xml.etree.ElementTree as ET
from collections import defaultdict

SPEC_RE = re.compile(
    r"treevariable\s+-set\s+var\.(?P<det>\w+)\.(?P<term>offset|slope|square)\.(?P<idx>\d+)\s+(?P<val>[-+0-9.eE]+)"
)

ORDER = "UBGR"
LABR_NUMBERS = [8, 12, 15, 18, 23]


def spectcl_key(det, idx):
    det = det.lower()

    if det == "clovercrystal":
        return ("clover", idx // 4 + 1, ORDER[idx % 4])

    if det == "labr3":
        labr_i = idx // 4
        if labr_i >= len(LABR_NUMBERS):
            return None
        return ("labr3", LABR_NUMBERS[labr_i], ORDER[idx % 4])

    return None


def build_xml_map(xml_path):
    root = ET.parse(xml_path).getroot()
    out = {}

    for crate in root.iter("Crate"):
        crate_num = int(crate.attrib["number"])

        for module in crate.iter("Module"):
            module_num = int(module.attrib["number"])

            for ch in module.iter("Channel"):
                ch_num = int(ch.attrib["number"])
                typ = ch.attrib.get("type", "").lower()
                subtype = ch.attrib.get("subtype", "")
                group = ch.attrib.get("group", "")
                tags = ch.attrib.get("tags", "")

                if typ == "clover":
                    key = ("clover", int(subtype), group.upper())
                    out[key] = (crate_num, module_num, ch_num)

                elif typ == "gscint" and subtype.lower() == "smallhag":
                    key = ("labr3", int(group), tags.upper())
                    out[key] = (crate_num, module_num, ch_num)

    return out


def write_cal_line(fh, crate, module, channel, coeffs):
    offset = coeffs["offset"]
    slope = coeffs["slope"]
    square = coeffs["square"]

    if square == 0.0:
        fh.write(f"{crate} {module} {channel} {offset} {slope}\n")
    else:
        fh.write(f"{crate} {module} {channel} {offset} {slope} {square}\n")


def main():
    ap = argparse.ArgumentParser(
        description="Convert SpecTcl calibration output to Pixie calibration format.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )

    ap.add_argument("-i", "--input", required=True, metavar="FILE",
                    help="SpecTcl calibration output file")
    ap.add_argument("-x", "--xml", required=True, metavar="FILE",
                    help="Detector map XML file")
    ap.add_argument("-o", "--output", required=True, metavar="FILE",
                    help="Output calibration file")

    args = ap.parse_args()

    xml_map = build_xml_map(args.xml)
    vals = defaultdict(lambda: {"offset": 0.0, "slope": 1.0, "square": 0.0})

    with open(args.input) as f:
        for line in f:
            m = SPEC_RE.search(line)
            if not m:
                continue

            det = m.group("det").lower()
            idx = int(m.group("idx"))
            term = m.group("term")
            val = float(m.group("val"))

            vals[(det, idx)][term] = val

    with open(args.output, "w") as fh:
        for (det, idx), coeffs in sorted(vals.items(), key=lambda x: (x[0][0], x[0][1])):
            key = spectcl_key(det, idx)

            if key is None:
                print(f"WARNING: no SpecTcl mapping for {det}.{idx:02d}", file=sys.stderr)
                continue

            if key not in xml_map:
                print(f"WARNING: no XML match for {det}.{idx:02d} -> {key}", file=sys.stderr)
                continue

            crate, module, channel = xml_map[key]
            write_cal_line(fh, crate, module, channel, coeffs)


if __name__ == "__main__":
    main()