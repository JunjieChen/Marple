import os,random,math
import datetime
import os.path
import subprocess as subp
import sys
import thread

def exccmd(cmd):
    p=os.popen(cmd,"r")
    rs=[]
    line=""
    while True:
         line=p.readline()
         if not line:
              break
         #print line
         #rs.append(line.strip())
    return rs
basicdir='/devdata/Junjie/CompilerFL/llvmforme/result'
locationdir='/home/Junjie/CompilerFL/locations/llvm'

revisions=['r247047','r243961']

result=open('FileMCMCCovOnlyLLVMnew2','a')
for rev in revisions:
	result.write(rev+':\n')
	locationfile=open(locationdir+'/'+rev[1:]+'/locations')
	locationlines=locationfile.readlines()
	locationfile.close()
	buggyfiles=set()
	for i in range(len(locationlines)):
		if 'file' in locationlines[i].strip() and 'method' in locationlines[i].strip():
			buggyfile='lib/'+locationlines[i].strip().split(';')[0].strip().split(':')[1].strip().split('/lib/')[1]
			buggyfiles.add(buggyfile)

	if os.path.exists(basicdir+'/'+rev+'-cov'+'/failcov/method_info.txt'):
		tarpath=basicdir+'/'+rev+'-cov'+'/failcov/method_info.txt'
	else:
		tarpath=basicdir+'/'+rev+'-cov'+'/fail/method_info.txt'
	failfile=open(tarpath)
	faillines=failfile.readlines()
	failfile.close()

	failmethod=dict()
	passmeancoverage=dict()
	for i in range(len(faillines)):
		faillinesplit=faillines[i].strip().split(',')
		filename=faillinesplit[0].strip().split('.gcda')[0].strip()
		if not filename.endswith('.cpp'):
			continue
		methodname=faillinesplit[1].strip()
		coverage=float(faillinesplit[2].strip())
		if filename not in failmethod.keys():
			failmethod[filename]=dict()
			passmeancoverage[filename]=dict()
		if methodname not in failmethod[filename].keys():
			failmethod[filename][methodname]=coverage
			passmeancoverage[filename][methodname]=0.0

	cnt=0
	for i in os.listdir(basicdir+'/'+rev+'-cov'+'/passcov'):
		# if int(i.strip().split('_')[-1].split('.')[0])>500:
		# 	continue
		cnt+=1
		print rev+' '+str(cnt)
		passfile=open(basicdir+'/'+rev+'-cov'+'/passcov/'+i+'/method_info.txt')
		passlines=passfile.readlines()
		passfile.close()
		selected=set()
		for j in range(len(passlines)):
			passlinesplit=passlines[j].strip().split(',')
			filename=passlinesplit[0].strip().split('.gcda')[0].strip()
			if not filename.endswith('.cpp'): # consider c and h files
				continue
			methodname=passlinesplit[1].strip()
			coverage=float(passlinesplit[2].strip())
			if filename+','+methodname not in selected:
				if filename in failmethod.keys() and methodname in failmethod[filename].keys():
					passmeancoverage[filename][methodname]+=abs(failmethod[filename][methodname]-coverage)/max(failmethod[filename][methodname],coverage)
				selected.add(filename+','+methodname)
		for fi in failmethod.keys():
			for me in failmethod[fi].keys():
				if fi+','+me not in selected:
		# for left in (set(failmethod.keys())-selected):
					passmeancoverage[fi][me]+=1.0

	score=dict()
	filescore=dict()
	for fi in failmethod.keys():
		score[fi]=0.0
		for me in failmethod[fi].keys():
			score[fi]+=passmeancoverage[fi][me]/500.0
		score[fi]/=len(failmethod[fi].keys())
	
	scorelist=sorted(score.items(),key=lambda d:d[1],reverse=True)

	for bf in buggyfiles:
		for i in range(len(scorelist)):
			setbf=set(bf.split('/'))
			seti=set(scorelist[i][0].split('/'))
			if setbf.issubset(seti):
				bf=scorelist[i][0]
				break
		tmp=[]
		for i in range(len(scorelist)):
			if score[bf]==scorelist[i][1]:
				tmp.append(i)
		result.write(bf+','+str(min(tmp))+','+str(max(tmp))+','+str(score[bf])+'\n')
	result.write('\n')
	result.flush()
result.close()


