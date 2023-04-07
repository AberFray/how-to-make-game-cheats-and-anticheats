# Discussion + Extra resources

## Resources

This repository is only meant as a "jumping off point" so to say. Something to get people interested and for them to find other places with more in-depth info.

Unfortunately, there isn't a large amount out there that goes further beyond what is covered here publicly, especially for anti-cheat. But that doesn't mean that there is none.

For the cheats side there are two main resources:
- [Gamehacking.academy](https://gamehacking.academy/ "Gamehacking.academy")
Similar to this resource, Game Hacking Academy comprises of multiple "Lessons" and "Labs" about game hacking. With it containing information on multiple different games and genres, as well as using a program debugger to find code and values inside games.
This though doesn't touch on anti-cheats, either developing or avoiding. And doesn't contain challenges for the user to improve or use their imagination, giving all the information directly.
- [Guidedhacking.com](https://guidedhacking.com/ "Guidedhacking.com")
Guided Hacking although public, it is paywalled to a certain extent. But does contain a large amount of in depth resources about game hacking along intricacies of different games and their anti-cheats. They also have their own YouTube channel that helps with learning and demonstrating tools, concepts and development. Additionally it covers multiple languages and reverse engineering in general.
Its interface however is a little more complicated to navigate and the tone of the content is not exactly professional. And it only covers anti-cheat bypassing and not development.

As for anti-cheat. There isn't really a central place with information about it, although places that show how to avoid anti-cheat could give you ideas how to counter their avoidance strategies.

[This paper](https://helda.helsinki.fi/bitstream/handle/10138/313587/Anti_cheat_for_video_games_final_07_03_2020.pdf?sequence=2&isAllowed=y "This paper") by Samuli Lehtonen for the University of Helsinki is a great paper that discusses the general situation with cheats and anti-cheats along with an overview of methods used anti-cheats.

There is also [NoMercy](https://github.com/mq1n/NoMercy "NoMercy"), a open source anti-cheat. Unfortunately it has been archived since 2019 and its documentation is lacking. But GitHub is a good place to look for resources for both sides, this is why this is on here.

## Server side VS Client side

This resource only covered client side cheats and anti-cheats which are the more commonly known and used types of cheats. But server side is important and useful to consider for both the offense and defence.

Server side cheats are usually for sending specially crafted packets to the server. Like constantly telling the server that you got a health pickup. Where the anti-cheat attempts to block this, in this example it would check if the player's location is in an area where they could get the health pickup. Some server side anti-cheat also attempts to catch cheaters if they are playing suspiciously.

The benefits of the anti-cheats is that they don't have to run on the user's machine. Which means it can be harder to bypass and impossible to tamper with. The drawbacks however are that it can't catch some cheats.

## A warning on kernel level anti-cheat development

As stated multiple times in the repository. Kernel level anti-cheat won't be taught. This isn't because of a dislike of the method, it's due to the lack of experience the writer has about it which crucially could lead to insecure code being used. Since anti-cheat drivers run on the highest privilege possible they need to be extra secure as exploiting a vulnerability in them could allow full access to the machine for a malicious user.

This happened with the driver `mhypro2.sys` used by the game Genshin Impact. With malware developers having their viruses install the driver onto victims computers and then exploiting their vulnerability to close anti-virus applications. More information can be found [here](https://www.trendmicro.com/en_us/research/22/h/ransomware-actor-abuses-genshin-impact-anti-cheat-driver-to-kill-antivirus.html "here").

A final point is that kernel level driver development is a complicated process, especially for Windows. So it wouldn't be great for an introductory resource for people.

## Does this interest you in malware?

Video game hacking and anti-hacking is quite similar to malware and anti-malware development (my anti-virus will attest to this). If this interests you, then maybe you could look more into this?

# Feedback welcome!

If you could send any **thoughts or feedback** about this resource [**here**](https://forms.office.com/e/r9Mdy3stif "Survey") then that would be very much appreciated.
You can also find the paper corresponding about this [here]({PaperLinkHere} "White paper").

Also if there is anything pressing you think needs changed then feel to submit an issue in via GitHub.

Thank you for taking your time to read this.

-Fraser
