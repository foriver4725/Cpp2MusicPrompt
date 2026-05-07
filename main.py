# type: ignore
import json


def load_events(path: str) -> list[dict]:
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def convert_event_to_music_text(event: dict) -> str:
    kind = event["kind"]
    name = event["name"]

    match kind:
        case "FunctionDefinition":
            return f"新しいテーマ開始: {name}"

        case "VariableDefinition":
            return f"短いモチーフ追加: {name}"

        case "IfStatement":
            return "緊張感のある分岐"

        case "ForLoop":
            return "反復リズム"

        case "WhileLoop":
            return "継続する不安定な展開"

        case "ReturnStatement":
            return "終止感"

        case "FunctionCall":
            return f"既存テーマ再利用: {name}"

        case "ScopeBlockStart":
            return "セクション開始"

        case "ScopeBlockEnd":
            return "セクション終了"

        case _:
            return f"未知イベント: {kind}"


def build_prompt(events: list[dict]) -> str:
    lines = []

    lines.append("以下のプログラム構造を音楽として表現してください。")
    lines.append("")

    for event in events:
        lines.append(convert_event_to_music_text(event))

    return "\n".join(lines)


def main():
    events = load_events("events.json")

    prompt = build_prompt(events)

    print("===== GENERATED PROMPT =====")
    print(prompt)


if __name__ == "__main__":
    main()
