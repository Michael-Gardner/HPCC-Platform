{
    "files": [{
        "aql": 
        {
            "items.find": 
            {
                "repo": "hpccpl-rpm-local",
                "$or": [
                    {"path": "LN/el7/x86_64"},
                    {"path": "LN/el8/x86_64"},
                    {"path": "LN/amzn2/x86_64"},
                    {"path": "LN/rocky8/x86_64"},
                    {"path": "EE/el7/x86_64"},
                    {"path": "EE/el8/x86_64"},
                    {"path": "EE/amzn2/x86_64"}
                ],
                "name": {"$match": "*-rc*"},
                "created": {"$before": "1mo"}
            }
        }
    }]
}
