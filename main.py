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
            return f"Introduce a new musical theme: {name}"

        case "VariableDefinition":
            return f"Add a short musical motif: {name}"

        case "IfStatement":
            return "Create a tense branching progression"

        case "ForLoop":
            return "Introduce a repetitive rhythmic pattern"

        case "WhileLoop":
            return "Create a continuously unstable development"

        case "ReturnStatement":
            return "Add a feeling of resolution"

        case "FunctionCall":
            return f"Reuse an existing musical theme: {name}"

        case "ScopeBlockStart":
            return "Begin a new musical section"

        case "ScopeBlockEnd":
            return "End the current musical section"

        case _:
            return f"Unknown event: {kind}"


def build_prompt(events: list[dict]) -> str:
    lines = []

    lines.append(
        "Please compose music that represents the following program structure."
    )
    lines.append(
        "Translate programming events into musical ideas, atmosphere, rhythm, and progression."
    )
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
