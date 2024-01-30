# Build Assets for individual developer

## Build Assets
The modern tool used for generating all our official assets is the Github Actions build-asset workflow on the hpcc-systems/HPCC-Platform repository, located [here](https://github.com/hpcc-systems/HPCC-Platform/actions/workflows/build-assets.yml).  But developers and contributors can utilize this same workflow on their own forked repository. This allows developers to quickly create assets for testing changes and test for build breaks before the peer review process.

Build assets will generate every available project under the HPCC-Platform namespace. There currently is not an option to control which packages in the build matrix get generated.  But most packages get built in parallel, and __released__ after the individual matrix job is completed, so there is no waiting on packages you don't need. Exceptions to this are for packages that require other builds to complete, such as the __ECLIDE__.

Upon completion of each step and matrix job in the workflow, the assets will be output to the repositories tags tab.  An example for the `hpcc-systems` user repository is [hpcc-systems/HPCC-Platform/tags](https://github.com/hpcc-systems/HPCC-Platform/tags).

![Tag tab screenshot](/devdoc/.assets/images/repository-tag-tab.png)

## Dependent variables
The build assets workflow requires several __repository secrets__ be available on a developers machine in order to run properly.  You can access these secrets and variables by going to the `settings` tab in your forked repository, and then clicking on the `Secrets and Variables - Actions` drop down under `Security` on the lefthand side of the settings screen.

![Actions secrets and variables](/devdoc/.assets/images/actions-secrets-and-variables.png)

Create a secret by clicking the green `New Repository Secret` button.  The following secrets are needed;

* LNB_ACTOR - Your Github username
* LNB_TOKEN - Classic Github token for your user with LN repo access
* DOCKER_USERNAME - Your docker.io username
* DOCKER_PASSWORD - Your docker.io password
* SIGNING_CERTIFICATE - pks12 self signed cert encoded to base64 for windows signing
* SIGNING_CERTIFICATE_PASSPHRASE - passphrase for pks12 cert
* SIGNING_SECRET - ssh-keygen private key for signing linux builds
* SIGN_MODULES_KEYID - email used to generate key
* SIGN_MODULES_PASSPHRASE - passphrase for private key