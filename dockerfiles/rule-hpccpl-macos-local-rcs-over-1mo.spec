{
    "files": [{
        "aql": 
        {
            "items.find": 
            {
                "repo": "hpccpl-macos-local",
                "path": "LN/macos/x86_64",
                "name": {"$match": "*-rc*"},
                "created": {"$before": "1mo"}
            }
        }
    }]
}
