import pandas as pd

class NFA:  #NFA Class 정의
    def __init__(self, state,insym,mapping,sstate,estate):
        self.states = state          #상태 유한 집합
        self.input_symbol = insym    #입력 심벌 유한 집합
        self.start_state = sstate    #시작상태 q0
        self.end_state = estate      #종결상태 F
        self.mapping_func = mapping  #사상 함수

    def print_nfa(self):    #NFA 출력 함수
        # 사상함수를 DataFrame 형태로 출력
        a = pd.DataFrame(self.mapping_func)

        print(f'Q -> {self.states}')
        print(f'∑ -> {self.input_symbol}')
        print(f'δ ->\n {a.transpose()}\n')
        print(f'q0 -> {self.start_state}')
        print(f'F -> {self.end_state}')

    def print_dfa(self,states,map_func,start,end):    #DFA 출력 함수
       a=pd.DataFrame(map_func)

       print(f'Q -> {states}')
       print(f'∑ -> {self.input_symbol}')
       print(f'δ ->\n {a.transpose()}\n')
       print(f'q0 -> {start}')
       print(f'F -> {end}')

    def print_rdfa(self,rdfa_states,rdfa_mapping_func,rdfa_start,rdfa_end):  #rdfa 출력
        a = pd.DataFrame(rdfa_mapping_func)

        print(f'Q -> {rdfa_states}')
        print(f'∑ -> {self.input_symbol}')
        print(f'δ ->\n {a.transpose()}\n')
        print(f'q0 -> {rdfa_start}')
        print(f'F -> {rdfa_end}')

    def NFA_TO_DFA(self):  #NFA to DFA

        remain_states = [] #아직 확인하지 못한 상태를 저장 (like Queue)
        states_dict= {}                     #상태의 dictioonary ex) S0 -> [0], S1 -> [0,1]...
        remain_index = 0                    #states_dict의 Index

        states_dict['S'+str(remain_index)] = [self.start_state] #Start state S0 = [0]
        remain_states.append('S0')
        d_start = {'S0':[self.start_state]}   #DFA의 시작상태 q0 정의
        d_end =  {}

        dfa_mapping_func = {}              #dfa의 mapping_function

        while(remain_states):              #Queue에 확인 할 state가 남아 있을 때 까지,

            now_states = states_dict.get(remain_states[0])   #현재 확인 할 State,
            tmp_dict = {}                                    #now_state와 매칭될 임시 dictionary

            for sym in self.input_symbol:                    #모든 Symbol에 대해,
                tmp_list = []                                #state를 저장할 임시 list

                for states in now_states:                    #모든 각각의 state들에 대해,
                    tmp_list+=self.mapping_func[states][sym] #NFA의 mapping_function을 통해, 상태 확인
                tmp_list=list(set(tmp_list))                 #중복되는 값 제거

                if None in tmp_list:                         #만약 None (empty set)이 존재 할 경우,
                    if not len(tmp_list)==1:                 #empty set이 아닌 경우, 즉 공집합이 아닌 경우,
                        for item in tmp_list:                #tmp_list에서 공집합을 삭제한다.
                            if item == None:
                                tmp_list.remove(None)

                tmp_dict[sym]=tmp_list                       #해당 심볼 일 때의 전이 state들을 저장

                if not (tmp_list in states_dict.values()):   #만약 현재 state가 새로운 state일 경우,
                    remain_index += 1                        #state의 index 증가 ex) S0 -> S1
                    states_dict['S' + str(remain_index)] = tmp_list  #현재 state를 추가.
                    remain_states.append('S' + str(remain_index)) #Queue에 state 추가


            for key,value in states_dict.items():            #현재 state의 이름을 찾기 위한 과정,
                if value == now_states:
                    dfa_mapping_func[key]=tmp_dict           #Sn -> tmp_dict 저장.
                    break

            del remain_states[0]                             #확인 한 state에 대해 dequeue


        for key,value in states_dict.items():
            if self.end_state in value:                     #DFA의 종결상태 F 정의
                d_end[key] = value

        return states_dict,dfa_mapping_func,d_start,d_end   #상태의 유한집합, 사상함수, 시작상태, 종결상태를 반환



    def DFA_TO_RDFA(self,DFA_states,DFA_mapping_func,DFA_start,DFA_end):  #DFA TO reduced DFA
        equi_class = [list(DFA_states.keys()-DFA_end.keys())]             #처음 종결상태와 종결상태가 아닌 집합들로 나눔.
        equi_class += [list(DFA_end.keys())]

        while(True):                                            
            equi_class_set = {}                                            
            for i in range(len(equi_class)):                              #각 상태들의 집합을 나타내는 SET 번호
                equi_class_set[i] = equi_class[i]

            new_equi_class=[]                                             #동치 관계를 확인 후 새로운 집합으로 나누기 위한 변수
            states_set = {}                                               #각 상태들의 사상함수
            states_complete = {}                                          #각 상태들의 사상함수(Symbol 포함 ver)

            for classes in equi_class:                                    #나누어진 각 부분집합에 대해,
                states_tmp = {}                                           #각 상태들의 전이상태를 저장하는 임시 딕셔너리

                for state in classes:                                     #각 부분 집합들의 모든 상태들에 대해,
                    tmp = []                                              #전이상태가 속하는 집합만 저장
                    tmp_dict = {}                                         #각 상태에 입력심볼에 따른 전이상태 저장 ex){a:1,b:3}
                    
                    for sym in self.input_symbol:                         #모든 입력 심벌에 대해,

                        for key,value in DFA_states.items():              #상태 유한집합에 대해,
                            if(value==DFA_mapping_func[state][sym]):      #해당 상태가 속하는 SET의 번호를 찾는다.
                                for set_key,set_value in equi_class_set.items():  
                                    if(key in set_value):                 #해당 번호를 찾았을 때,
                                        tmp.append(set_key)               #전이상태가 속하는 집합을 tmp에 append
                                        tmp_dict[sym] = set_key           #입력 심볼에 대한 전이상태의 set 번호 딕셔너리로 저장


                    states_tmp[state] = tmp                               #상태 임시저장
                    states_set[state] = tmp                               #전체 전이상태만 저장. (입력심벌 제외)
                    states_complete[state] = tmp_dict                     #임력심벌을 포함한 전체 전이상태 저장
                    
                for equi in list(map(list,list(set(map(tuple,states_tmp.values()))))):  #각 부분집합에 대한 동치를 확인하기 위함.
                    tmp_list = []
                    for key, value in states_tmp.items():                               #동치인 부분 끼리 분류해서 다시 부분집합을 만드는 과정
                       if(value==equi):
                           tmp_list.append(key)
                    new_equi_class.append(tmp_list)


            if(equi_class==new_equi_class):                               #만약 이전 동치집합과 새로운 동치 집합이 같다면, 동치 분류가 끝난 것이므로 loop 탈출
                break
            else:
                equi_class = new_equi_class                               #새로 달라진 부분집합에 대해 다시 전이상태 확인 loop 반복



        index = 0                                                         #rdfa의 상태를 치환하기 위한 index
        rdfa_states = {}                                                  #rdfa 상태의 유한집함
        rdfa_start = {}                                                   #rdfa 의 시작 상태 q0
        rdfa_end = {}                                                     #rdfa 의 종결 상태 F


        for a in equi_class:                                              #동치 분류 작업이 끝난 부분집합들에 대해
            if list(DFA_start.keys())[0] in a:                            #새로운 rdfa 상태 유한집합으로 치환 작업
                rdfa_start['R'+str(index)] = a                            #ex) [S0,S1] -> R1...

            for end in DFA_end.keys():                                    #rdfa의 종결 상태 정의
                if end in a:                                              #dfa의 종결 상태가 하나라도 포함되면,
                    rdfa_end['R'+str(index)]=a                            #rdfa의 종결 상태가 됨.
                    
            rdfa_states['R'+str(index)] = a                               #상태의 유한집합 정의
            index+=1

        rdfa_mapping_func = {}                                            #사상함수 정의

        for key,value in states_complete.items():                         #기존의 DFA의 상태들을 새로운 상태로 치환하기 위한 과정
                                                                          #ex) [S0,S1],a -> [S2]  / [R1],a -> [R2]
                                                                          #     [S0,S1] = R1 , [S2] = R2
            t_dict = {}

            for r_key,r_value in rdfa_states.items():
                if [key] == r_value:
                    for sym in self.input_symbol:
                        for rr_key,rr_value in rdfa_states.items():

                            if rr_value == equi_class_set[value[sym]]:
                                t_dict[sym]=rr_key
                    rdfa_mapping_func[r_key]=t_dict

        return rdfa_states, rdfa_mapping_func, rdfa_start, rdfa_end     #rdfa 상태의 유한집함, 사상함수, 시작 및 종결 상태 반환




#================================================================================================================================

#======================================== NFA TO DFA Convert Start ==============================================================

#mapping func 정의
mapping_func = \
            {0:
                 {'a':[0,1],
                  'b':[0]}
                ,
             1:
                 {'a':[None],
                  'b':[2]},
             2:
                 {'a': [None],
                  'b': [3]},
             3:
                 {'a': [None],
                  'b': [None]},
             }
#NFA 객체 생성(상태의 유한집합,Input_Symbol,Start_state,End_state)
a= NFA([0,1,2,3],['a','b'],mapping_func,0,3)
print("NFA : ")
a.print_nfa()                      #NFA 출력
print("==============================================================")
                   
DFA_states,DFA_mapping_func,DFA_start,DFA_end = a.NFA_TO_DFA()  #NFA TO DFA 실행

print("DFA : ")
a.print_dfa(DFA_states,DFA_mapping_func,DFA_start,DFA_end)      #DFA 출력
print("==============================================================")

rdfa_states,rdfa_mapping_func,rdfa_start,rdfa_end=a.DFA_TO_RDFA(DFA_states,DFA_mapping_func,DFA_start,DFA_end) #DFA TO rDFA 실행
print("rDFA : ")
a.print_rdfa(rdfa_states,rdfa_mapping_func,rdfa_start,rdfa_end) #rDFA 출력















