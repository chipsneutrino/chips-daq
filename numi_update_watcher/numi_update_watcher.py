#!/usr/bin/env python3

import http.client
import re
import pprint
import time

GET_SERVER = 'www-bd.fnal.gov'
GET_PATH = '/notifyservlet/www'
REFRESH_INTERVAL_SEC = 1
RE_OPTS = re.IGNORECASE

###

RE_DATE_GENERATED = re.compile('<\/td>\n<td align=right>([a-zA-Z0-9: -]+)<\/td>\n<\/tr><\/table>', RE_OPTS)
RE_COMPLEX_STATE = re.compile('<td>Complex</td><td>([^<]*)</td><td>([^<]*)</td>', RE_OPTS)
RE_MAIN_INJECTOR_STATE = re.compile('<td><a[^>]*>MainInjector</a></td><td>([^<]*)</td><td>([^<]*)</td>', RE_OPTS)
RE_RECYCLER_STATE = re.compile('<td>Recycler</td><td>([^<]*)</td><td>([^<]*)</td>', RE_OPTS)
RE_TEMPERATURE = re.compile('<tr><td>Temperature</td><td align=right><b>([-+E0-9.]+)</b></td><td align=left>[^<]*</td></tr>', RE_OPTS)

def table_re(field_name):
    return re.compile('<td[^>]*>%s</td><td[^>]* align=right>[<b>]*([-+E0-9.]*)[</b>]*</td><td align=left>([^<]+)</td>' % (field_name), RE_OPTS)

RE_NUMI_BEAM = table_re('NuMI Beam')
RE_NUMI_POWER = table_re('NuMI Power')
RE_BNB_RATE = table_re('BNB Rate')
RE_BNB1D_RATE = table_re('BNB 1D Rate')
RE_SY_TOTAL = table_re('SY Total')
RE_M_TEST = table_re('MTest')
RE_M_CENTER = table_re('MCenter')
RE_NM = table_re('NM')
RE_SOURCE_BEAM = table_re('Source Beam')
RE_LINAC_BEAM = table_re('Linac Beam')
RE_BOOSTER_BEAM = table_re('Booster Beam</a>')
RE_RECYCLER = table_re('Recycler</a>')
RE_MI_BEAM = table_re('MI Beam')
RE_SRC_STAT = re.compile('<td>SRC Stat</td><td align=right>([^<]+)</td><td align=left></td>', RE_OPTS)
RE_RATE = table_re('Rate')
RE_MUON_POT = table_re('Muon POT')

RE_IMPORTANT_MESSAGE = re.compile('<p><font color=red><pre>([^<]*)</pre></font>', RE_OPTS)
RE_MESSAGE = re.compile('<p><pre>([^<]*)</pre>', RE_OPTS)

###

def request_data():
    connection = http.client.HTTPSConnection(GET_SERVER)
    connection.request('GET', GET_PATH)
    response = connection.getresponse()

    if response.status != 200:
        connection.close()
        raise Exception('Unexpected response code: %d' % response.status)
    
    data = response.read().decode('utf-8')
    connection.close()
    return data

def extract_features(data):
    def match_single(text, regexp, default=None):
        matches = re.findall(regexp, text)
        return matches[0] if len(matches) == 1 else default

    def apply_if_some(optional, fn):
        return fn(optional) if optional is not None else None
    
    def strip_if_some(optional):
        return apply_if_some(optional, lambda s: s.strip())

    def float_or_none(float_str):
        try:
            return float(float_str)
        except ValueError:
            return None

    def float_first(pair):
        num, unit = pair
        return apply_if_some(num, float_or_none), unit

    features = {}

    features['dateGenerated'] = match_single(data, RE_DATE_GENERATED)

    features['complexState'], features['complexLastDateChanged'] = match_single(data, RE_COMPLEX_STATE, default=(None, None))
    features['mainInjectorState'], features['mainInjectorLastDateChanged'] = match_single(data, RE_MAIN_INJECTOR_STATE, default=(None, None))
    features['recyclerState'], features['recyclerLastDateChanged'] = match_single(data, RE_RECYCLER_STATE, default=(None, None))

    features['temperature'] = float_or_none(match_single(data, RE_TEMPERATURE))
    features['numiBeam'], features['numiBeamUnit'] = float_first(match_single(data, RE_NUMI_BEAM, default=(None, None)))
    features['numiPower'], features['numiPowerUnit'] = float_first(match_single(data, RE_NUMI_POWER, default=(None, None)))
    features['bnbRate'], features['bnbRateUnit'] = float_first(match_single(data, RE_BNB_RATE, default=(None, None)))
    features['bnb1dRate'], features['bnb1dRateUnit'] = float_first(match_single(data, RE_BNB1D_RATE, default=(None, None)))

    features['syTotal'], features['syTotalUnit'] = float_first(match_single(data, RE_SY_TOTAL, default=(None, None)))
    features['mTest'], features['mTestUnit'] = float_first(match_single(data, RE_M_TEST, default=(None, None)))
    features['mCenter'], features['mCenterUnit'] = float_first(match_single(data, RE_M_CENTER, default=(None, None)))
    features['nm'], features['nmUnit'] = float_first(match_single(data, RE_NM, default=(None, None)))

    features['sourceBeam'], features['sourceBeamUnit'] = float_first(match_single(data, RE_SOURCE_BEAM, default=(None, None)))
    features['linacBeam'], features['linacBeamUnit'] = float_first(match_single(data, RE_LINAC_BEAM, default=(None, None)))
    features['boosterBeam'], features['boosterBeamUnit'] = float_first(match_single(data, RE_BOOSTER_BEAM, default=(None, None)))
    features['recycler'], features['recyclerUnit'] = float_first(match_single(data, RE_RECYCLER, default=(None, None)))
    features['miBeam'], features['miBeamUnit'] = float_first(match_single(data, RE_MI_BEAM, default=(None, None)))

    features['srcStat'] = strip_if_some(match_single(data, RE_SRC_STAT))
    features['rate'], features['rateUnit'] = float_first(match_single(data, RE_RATE, default=(None, None)))
    features['muonPot'], features['muonPotUnit'] = float_first(match_single(data, RE_MUON_POT, default=(None, None)))

    features['importantMessage'] = strip_if_some(match_single(data, RE_IMPORTANT_MESSAGE))
    features['message'] = strip_if_some(match_single(data, RE_MESSAGE))

    return features

def main():
    running = True
    while running:
        try:
            data = request_data()
            features = extract_features(data)
            pp = pprint.PrettyPrinter(indent=4)
            pp.pprint(features)

            time.sleep(REFRESH_INTERVAL_SEC)
        except KeyboardInterrupt:
            running = False

if __name__ == '__main__':
    main()
