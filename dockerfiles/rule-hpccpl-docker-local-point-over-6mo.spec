{
"files": [{
    "aql": {
        "items.find": {
            "$or":
            [
                {
                    "$and":
                    [
                        {"repo": "hpccpl-docker-local"},
                        {"type": "folder"},
                        {"name": {"$match": "*8.6*"}},
                        {"created": {"$before": "6mo"}}
                    ]
                },
                {
                    "$and":
                    [
                        {"repo": "hpccpl-docker-local"},
                        {"type": "folder"},
                        {"name": {"$match": "8.8*"}},
                        {"created": {"$before": "6mo"}}
                    ]
                },
                {
                    "$and":
                    [
                        {"repo": "hpccpl-docker-local"},
                        {"type": "folder"},
                        {"name": {"$match": "8.10*"}},
                        {"created": {"$before": "6mo"}}
                    ]
                },
                {
                    "$and":
                    [
                        {"repo": "hpccpl-docker-local"},
                        {"type": "folder"},
                        {"name": {"$match": "8.12*"}},
                        {"created": {"$before": "6mo"}}
                    ]
                },
                {
                    "$and":
                    [
                        {"repo": "hpccpl-docker-local"},
                        {"type": "folder"},
                        {"name": {"$match": "9.0*"}},
                        {"created": {"$before": "6mo"}}
                    ]
                },
                {
                    "$and":
                    [
                        {"repo": "hpccpl-docker-local"},
                        {"type": "folder"},
                        {"name": {"$match": "9.2"}},
                        {"created": {"$before": "6mo"}}
                    ]
                },
                {
                    "$and":
                    [
                        {"repo": "hpccpl-docker-local"},
                        {"type": "folder"},
                        {"name": {"$match": "9.4*"}},
                        {"created": {"$before": "6mo"}}
                    ]
                },
                {
                    "$and":
                    [
                        {"repo": "hpccpl-docker-local"},
                        {"type": "folder"},
                        {"name": {"$match": "9.6*"}},
                        {"created": {"$before": "6mo"}}
                    ]
                }
            ]
        }
    }
}]
}